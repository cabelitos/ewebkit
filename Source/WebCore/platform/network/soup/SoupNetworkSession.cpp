/*
 * Copyright (C) 2014 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if USE(SOUP)

#include "SoupNetworkSession.h"

#include "AuthenticationChallenge.h"
#include "FileSystem.h"
#include "GUniquePtrSoup.h"
#include "Logging.h"
#include "ResourceHandle.h"
#include "SoupNetworkProxySettings.h"
#include <glib/gstdio.h>
#include <libsoup/soup.h>
#include <pal/crypto/CryptoDigest.h>
#include <wtf/HashSet.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/Base64.h>
#include <wtf/text/CString.h>

namespace WebCore {

static bool gIgnoreTLSErrors;
static CString gInitialAcceptLanguages;
static SoupNetworkProxySettings gProxySettings;
static GType gCustomProtocolRequestType;

#if !LOG_DISABLED
inline static void soupLogPrinter(SoupLogger*, SoupLoggerLogLevel, char direction, const char* data, gpointer)
{
    LOG(Network, "%c %s", direction, data);
}
#endif

class HostTLSCertificateSet {
public:
    void add(GTlsCertificate* certificate)
    {
        String certificateHash = computeCertificateHash(certificate);
        if (!certificateHash.isEmpty())
            m_certificates.add(certificateHash);
    }

    bool contains(GTlsCertificate* certificate) const
    {
        return m_certificates.contains(computeCertificateHash(certificate));
    }

private:
    static String computeCertificateHash(GTlsCertificate* certificate)
    {
        GRefPtr<GByteArray> certificateData;
        g_object_get(G_OBJECT(certificate), "certificate", &certificateData.outPtr(), nullptr);
        if (!certificateData)
            return String();

        auto digest = PAL::CryptoDigest::create(PAL::CryptoDigest::Algorithm::SHA_256);
        digest->addBytes(certificateData->data, certificateData->len);

        auto hash = digest->computeHash();
        return base64Encode(reinterpret_cast<const char*>(hash.data()), hash.size());
    }

    HashSet<String> m_certificates;
};

static HashMap<String, HostTLSCertificateSet, ASCIICaseInsensitiveHash>& clientCertificates()
{
    static NeverDestroyed<HashMap<String, HostTLSCertificateSet, ASCIICaseInsensitiveHash>> certificates;
    return certificates;
}

static void authenticateCallback(SoupSession*, SoupMessage* soupMessage, SoupAuth* soupAuth, gboolean retrying)
{
    RefPtr<ResourceHandle> handle = static_cast<ResourceHandle*>(g_object_get_data(G_OBJECT(soupMessage), "handle"));
    if (!handle)
        return;
    handle->didReceiveAuthenticationChallenge(AuthenticationChallenge(soupMessage, soupAuth, retrying, handle.get()));
}

#if ENABLE(WEB_TIMING) && !SOUP_CHECK_VERSION(2, 49, 91)
static void requestStartedCallback(SoupSession*, SoupMessage* soupMessage, SoupSocket*, gpointer)
{
    RefPtr<ResourceHandle> handle = static_cast<ResourceHandle*>(g_object_get_data(G_OBJECT(soupMessage), "handle"));
    if (!handle)
        return;
    handle->didStartRequest();
}
#endif

SoupNetworkSession::SoupNetworkSession(SoupCookieJar* cookieJar)
    : m_soupSession(adoptGRef(soup_session_async_new()))
{
    // Values taken from http://www.browserscope.org/ following
    // the rule "Do What Every Other Modern Browser Is Doing". They seem
    // to significantly improve page loading time compared to soup's
    // default values.
    static const int maxConnections = 17;
    static const int maxConnectionsPerHost = 6;

    GRefPtr<SoupCookieJar> jar = cookieJar;
    if (!jar) {
        jar = adoptGRef(soup_cookie_jar_new());
        soup_cookie_jar_set_accept_policy(jar.get(), SOUP_COOKIE_JAR_ACCEPT_NO_THIRD_PARTY);
    }

    g_object_set(m_soupSession.get(),
        SOUP_SESSION_MAX_CONNS, maxConnections,
        SOUP_SESSION_MAX_CONNS_PER_HOST, maxConnectionsPerHost,
        SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_DECODER,
        SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_CONTENT_SNIFFER,
        SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_PROXY_RESOLVER_DEFAULT,
        SOUP_SESSION_ADD_FEATURE, jar.get(),
        SOUP_SESSION_USE_THREAD_CONTEXT, TRUE,
        SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
        SOUP_SESSION_SSL_STRICT, FALSE,
        nullptr);

    setupCustomProtocols();

    if (!gInitialAcceptLanguages.isNull())
        setAcceptLanguages(gInitialAcceptLanguages);

#if SOUP_CHECK_VERSION(2, 53, 92)
    if (soup_auth_negotiate_supported()) {
        g_object_set(m_soupSession.get(),
            SOUP_SESSION_ADD_FEATURE_BY_TYPE, SOUP_TYPE_AUTH_NEGOTIATE,
            nullptr);
    }
#endif

    if (gProxySettings.mode != SoupNetworkProxySettings::Mode::Default)
        setupProxy();
    setupLogger();

    g_signal_connect(m_soupSession.get(), "authenticate", G_CALLBACK(authenticateCallback), nullptr);
#if ENABLE(WEB_TIMING) && !SOUP_CHECK_VERSION(2, 49, 91)
    g_signal_connect(m_soupSession.get(), "request-started", G_CALLBACK(requestStartedCallback), nullptr);
#endif
}

SoupNetworkSession::~SoupNetworkSession()
{
}

void SoupNetworkSession::setupLogger()
{
#if !LOG_DISABLED
    if (LogNetwork.state != WTFLogChannelOn || soup_session_get_feature(m_soupSession.get(), SOUP_TYPE_LOGGER))
        return;

    GRefPtr<SoupLogger> logger = adoptGRef(soup_logger_new(SOUP_LOGGER_LOG_BODY, -1));
    soup_session_add_feature(m_soupSession.get(), SOUP_SESSION_FEATURE(logger.get()));
    soup_logger_set_printer(logger.get(), soupLogPrinter, nullptr, nullptr);
#endif
}

void SoupNetworkSession::setCookieJar(SoupCookieJar* jar)
{
    if (SoupCookieJar* currentJar = cookieJar())
        soup_session_remove_feature(m_soupSession.get(), SOUP_SESSION_FEATURE(currentJar));
    soup_session_add_feature(m_soupSession.get(), SOUP_SESSION_FEATURE(jar));
}

SoupCookieJar* SoupNetworkSession::cookieJar() const
{
    return SOUP_COOKIE_JAR(soup_session_get_feature(m_soupSession.get(), SOUP_TYPE_COOKIE_JAR));
}

static inline bool stringIsNumeric(const char* str)
{
    while (*str) {
        if (!g_ascii_isdigit(*str))
            return false;
        str++;
    }
    return true;
}

// Old versions of WebKit created this cache.
void SoupNetworkSession::clearOldSoupCache(const String& cacheDirectory)
{
    CString cachePath = fileSystemRepresentation(cacheDirectory);
    GUniquePtr<char> cacheFile(g_build_filename(cachePath.data(), "soup.cache2", nullptr));
    if (!g_file_test(cacheFile.get(), G_FILE_TEST_IS_REGULAR))
        return;

    GUniquePtr<GDir> dir(g_dir_open(cachePath.data(), 0, nullptr));
    if (!dir)
        return;

    while (const char* name = g_dir_read_name(dir.get())) {
        if (!g_str_has_prefix(name, "soup.cache") && !stringIsNumeric(name))
            continue;

        GUniquePtr<gchar> filename(g_build_filename(cachePath.data(), name, nullptr));
        if (g_file_test(filename.get(), G_FILE_TEST_IS_REGULAR))
            g_unlink(filename.get());
    }
}

void SoupNetworkSession::setupProxy()
{
    GRefPtr<GProxyResolver> resolver;
    switch (gProxySettings.mode) {
    case SoupNetworkProxySettings::Mode::Default: {
        GRefPtr<GProxyResolver> currentResolver;
        g_object_get(m_soupSession.get(), SOUP_SESSION_PROXY_RESOLVER, &currentResolver.outPtr(), nullptr);
        GProxyResolver* defaultResolver = g_proxy_resolver_get_default();
        if (currentResolver.get() == defaultResolver)
            return;
        resolver = defaultResolver;
        break;
    }
    case SoupNetworkProxySettings::Mode::NoProxy:
        // Do nothing in this case, resolver is nullptr so that when set it will disable proxies.
        break;
    case SoupNetworkProxySettings::Mode::Custom:
        resolver = adoptGRef(g_simple_proxy_resolver_new(nullptr, nullptr));
        if (!gProxySettings.defaultProxyURL.isNull())
            g_simple_proxy_resolver_set_default_proxy(G_SIMPLE_PROXY_RESOLVER(resolver.get()), gProxySettings.defaultProxyURL.data());
        if (gProxySettings.ignoreHosts)
            g_simple_proxy_resolver_set_ignore_hosts(G_SIMPLE_PROXY_RESOLVER(resolver.get()), gProxySettings.ignoreHosts.get());
        for (const auto& iter : gProxySettings.proxyMap)
            g_simple_proxy_resolver_set_uri_proxy(G_SIMPLE_PROXY_RESOLVER(resolver.get()), iter.key.data(), iter.value.data());
        break;
    }

    g_object_set(m_soupSession.get(), SOUP_SESSION_PROXY_RESOLVER, resolver.get(), nullptr);
    soup_session_abort(m_soupSession.get());
}

#if PLATFORM(EFL)
// FIXME: This function should not exist at all and we don't want to accidentally use it in other ports.
// The correct way to set proxy settings from the environment is to use a GProxyResolver that does so.
// It also lacks the rather important https_proxy and ftp_proxy variables, and the uppercase versions of
// all four variables, all of which you almost surely want to be respected if you're setting http_proxy,
// and all of which would be supported via the default proxy resolver in non-GNOME/Ubuntu environments
// (at least, I think that's right). Additionally, it is incorrect for WebKit to respect this environment
// variable when running in a GNOME or Ubuntu environment, where GNOME proxy configuration should be
// respected instead. The only reason to retain this function is to not alter the incorrect behavior for EFL.
void SoupNetworkSession::setProxySettingsFromEnvironment()
{
    const char* httpProxy = getenv("http_proxy");
    if (!httpProxy)
        return;

    gProxySettings.defaultProxyURL = httpProxy;
    const char* httpProxyExceptions = getenv("no_proxy");
    if (httpProxyExceptions)
        gProxySettings.ignoreHosts.reset(g_strsplit(httpProxyExceptions, ",", -1));
}
#endif

void SoupNetworkSession::setProxySettings(const SoupNetworkProxySettings& settings)
{
    gProxySettings = settings;
}

void SoupNetworkSession::setInitialAcceptLanguages(const CString& languages)
{
    gInitialAcceptLanguages = languages;
}

void SoupNetworkSession::setAcceptLanguages(const CString& languages)
{
    g_object_set(m_soupSession.get(), "accept-language", languages.data(), nullptr);
}

void SoupNetworkSession::setCustomProtocolRequestType(GType requestType)
{
    ASSERT(g_type_is_a(requestType, SOUP_TYPE_REQUEST));
    gCustomProtocolRequestType = requestType;
}

void SoupNetworkSession::setupCustomProtocols()
{
    if (!g_type_is_a(gCustomProtocolRequestType, SOUP_TYPE_REQUEST))
        return;

    auto* requestClass = static_cast<SoupRequestClass*>(g_type_class_peek(gCustomProtocolRequestType));
    if (!requestClass || !requestClass->schemes)
        return;

    soup_session_add_feature_by_type(m_soupSession.get(), gCustomProtocolRequestType);
}

void SoupNetworkSession::setShouldIgnoreTLSErrors(bool ignoreTLSErrors)
{
    gIgnoreTLSErrors = ignoreTLSErrors;
}

void SoupNetworkSession::checkTLSErrors(SoupRequest* soupRequest, SoupMessage* message, std::function<void (const ResourceError&)>&& completionHandler)
{
    if (gIgnoreTLSErrors) {
        completionHandler({ });
        return;
    }

    GTlsCertificate* certificate = nullptr;
    GTlsCertificateFlags tlsErrors = static_cast<GTlsCertificateFlags>(0);
    soup_message_get_https_status(message, &certificate, &tlsErrors);
    if (!tlsErrors) {
        completionHandler({ });
        return;
    }

    URL url(soup_request_get_uri(soupRequest));
    auto it = clientCertificates().find(url.host());
    if (it != clientCertificates().end() && it->value.contains(certificate)) {
        completionHandler({ });
        return;
    }

    completionHandler(ResourceError::tlsError(soupRequest, tlsErrors, certificate));
}

void SoupNetworkSession::allowSpecificHTTPSCertificateForHost(const CertificateInfo& certificateInfo, const String& host)
{
    clientCertificates().add(host, HostTLSCertificateSet()).iterator->value.add(certificateInfo.certificate());
}

} // namespace WebCore

#endif
