/*
 * Copyright (C) 2011, 2012 Igalia, S.L.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "GLContextGLX.h"

#if USE(GLX)
#include "GraphicsContext3D.h"
#include "OpenGLShims.h"
#include "PlatformDisplayX11.h"
#include "XErrorTrapper.h"
#include <GL/glx.h>
#include <cairo.h>

#if ENABLE(ACCELERATED_2D_CANVAS)
#include <cairo-gl.h>
#endif

namespace WebCore {

#if !defined(PFNGLXSWAPINTERVALSGIPROC)
typedef int (*PFNGLXSWAPINTERVALSGIPROC) (int);
#endif
#if !defined(PFNGLXCREATECONTEXTATTRIBSARBPROC)
typedef GLXContext (*PFNGLXCREATECONTEXTATTRIBSARBPROC) (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);
#endif

static PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI;
static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;

static bool hasSGISwapControlExtension(Display* display)
{
    static bool initialized = false;
    if (initialized)
        return !!glXSwapIntervalSGI;

    initialized = true;
    if (!GLContext::isExtensionSupported(glXQueryExtensionsString(display, 0), "GLX_SGI_swap_control"))
        return false;

    glXSwapIntervalSGI = reinterpret_cast<PFNGLXSWAPINTERVALSGIPROC>(glXGetProcAddress(reinterpret_cast<const unsigned char*>("glXSwapIntervalSGI")));
    return !!glXSwapIntervalSGI;
}

static bool hasGLXARBCreateContextExtension(Display* display)
{
    static bool initialized = false;
    if (initialized)
        return !!glXCreateContextAttribsARB;

    initialized = true;
    if (!GLContext::isExtensionSupported(glXQueryExtensionsString(display, 0), "GLX_ARB_create_context"))
        return false;

    glXCreateContextAttribsARB = reinterpret_cast<PFNGLXCREATECONTEXTATTRIBSARBPROC>(glXGetProcAddress(reinterpret_cast<const unsigned char*>("glXCreateContextAttribsARB")));
    return !!glXCreateContextAttribsARB;
}

static GLXContext createGLXARBContext(Display* display, GLXFBConfig config, GLXContext sharingContext)
{
    // We want to create a context with version >= 3.2 core profile, cause that ensures that the i965 driver won't
    // use the software renderer. If that doesn't work, we will use whatever version available. Unfortunately,
    // there's no way to know whether glXCreateContextAttribsARB can provide an OpenGL version >= 3.2 until
    // we actually call it and check the return value. To make things more fun, if a version >= 3.2 cannot be
    // provided, glXCreateContextAttribsARB will throw a GLXBadFBConfig X error, causing the app to crash.
    // So, the first time a context is requested, we set a X error trap to disable crashes with GLXBadFBConfig
    // and then check whether the return value is a context or not.

    static bool canCreate320Context = false;
    static bool canCreate320ContextInitialized = false;

    static const int contextAttributes[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 2,
        0
    };

    if (!canCreate320ContextInitialized) {
        canCreate320ContextInitialized = true;

        {
            // Set an X error trapper that ignores errors to avoid crashing on GLXBadFBConfig. Use a scope
            // here to limit the error trap to just this context creation call.
            XErrorTrapper trapper(display, XErrorTrapper::Policy::Ignore);
            GLXContext context = glXCreateContextAttribsARB(display, config, sharingContext, GL_TRUE, contextAttributes);
            if (context) {
                canCreate320Context = true;
                return context;
            }
        }

        // Creating the 3.2 context failed, so use whatever is available.
        return glXCreateContextAttribsARB(display, config, sharingContext, GL_TRUE, nullptr);
    }

    if (canCreate320Context)
        return glXCreateContextAttribsARB(display, config, sharingContext, GL_TRUE, contextAttributes);

    return glXCreateContextAttribsARB(display, config, sharingContext, GL_TRUE, nullptr);
}

std::unique_ptr<GLContextGLX> GLContextGLX::createWindowContext(GLNativeWindowType window, PlatformDisplay& platformDisplay, GLXContext sharingContext)
{
    Display* display = downcast<PlatformDisplayX11>(platformDisplay).native();
    XWindowAttributes attributes;
    if (!XGetWindowAttributes(display, static_cast<Window>(window), &attributes))
        return nullptr;

    XVisualInfo visualInfo;
    visualInfo.visualid = XVisualIDFromVisual(attributes.visual);

    int numConfigs = 0;
    GLXFBConfig config = nullptr;
    XUniquePtr<GLXFBConfig> configs(glXGetFBConfigs(display, DefaultScreen(display), &numConfigs));
    for (int i = 0; i < numConfigs; i++) {
        XUniquePtr<XVisualInfo> glxVisualInfo(glXGetVisualFromFBConfig(display, configs.get()[i]));
        if (!glxVisualInfo)
            continue;

        if (glxVisualInfo.get()->visualid == visualInfo.visualid) {
            config = configs.get()[i];
            break;
        }
    }
    ASSERT(config);

    XUniqueGLXContext context;
    if (hasGLXARBCreateContextExtension(display))
        context.reset(createGLXARBContext(display, config, sharingContext));
    else {
        // Legacy OpenGL version.
        XUniquePtr<XVisualInfo> visualInfoList(glXGetVisualFromFBConfig(display, config));
        context.reset(glXCreateContext(display, visualInfoList.get(), sharingContext, True));
    }

    if (!context)
        return nullptr;

    return std::unique_ptr<GLContextGLX>(new GLContextGLX(platformDisplay, WTFMove(context), window));
}

std::unique_ptr<GLContextGLX> GLContextGLX::createPbufferContext(PlatformDisplay& platformDisplay, GLXContext sharingContext)
{
    static const int fbConfigAttributes[] = {
        GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_ALPHA_SIZE, 1,
        GLX_DOUBLEBUFFER, GL_FALSE,
        0
    };

    int returnedElements;
    Display* display = downcast<PlatformDisplayX11>(platformDisplay).native();
    XUniquePtr<GLXFBConfig> configs(glXChooseFBConfig(display, 0, fbConfigAttributes, &returnedElements));
    if (!returnedElements)
        return nullptr;

    // We will be rendering to a texture, so our pbuffer does not need to be large.
    static const int pbufferAttributes[] = { GLX_PBUFFER_WIDTH, 1, GLX_PBUFFER_HEIGHT, 1, 0 };
    XUniqueGLXPbuffer pbuffer(glXCreatePbuffer(display, configs.get()[0], pbufferAttributes));
    if (!pbuffer)
        return nullptr;

    XUniqueGLXContext context;
    if (hasGLXARBCreateContextExtension(display))
        context.reset(createGLXARBContext(display, configs.get()[0], sharingContext));
    else {
        // Legacy OpenGL version.
        context.reset(glXCreateNewContext(display, configs.get()[0], GLX_RGBA_TYPE, sharingContext, GL_TRUE));
    }

    if (!context)
        return nullptr;

    return std::unique_ptr<GLContextGLX>(new GLContextGLX(platformDisplay, WTFMove(context), WTFMove(pbuffer)));
}

std::unique_ptr<GLContextGLX> GLContextGLX::createPixmapContext(PlatformDisplay& platformDisplay, GLXContext sharingContext)
{
    static int visualAttributes[] = {
        GLX_RGBA,
        GLX_RED_SIZE, 1,
        GLX_GREEN_SIZE, 1,
        GLX_BLUE_SIZE, 1,
        GLX_ALPHA_SIZE, 1,
        0
    };

    Display* display = downcast<PlatformDisplayX11>(platformDisplay).native();
    XUniquePtr<XVisualInfo> visualInfo(glXChooseVisual(display, DefaultScreen(display), visualAttributes));
    if (!visualInfo)
        return nullptr;

    XUniqueGLXContext context(glXCreateContext(display, visualInfo.get(), sharingContext, GL_TRUE));
    if (!context)
        return nullptr;

    XUniquePixmap pixmap(XCreatePixmap(display, DefaultRootWindow(display), 1, 1, visualInfo->depth));
    if (!pixmap)
        return nullptr;

    XUniqueGLXPixmap glxPixmap(glXCreateGLXPixmap(display, visualInfo.get(), pixmap.get()));
    if (!glxPixmap)
        return nullptr;

    return std::unique_ptr<GLContextGLX>(new GLContextGLX(platformDisplay, WTFMove(context), WTFMove(pixmap), WTFMove(glxPixmap)));
}

std::unique_ptr<GLContextGLX> GLContextGLX::createContext(GLNativeWindowType window, PlatformDisplay& platformDisplay)
{
    GLXContext glxSharingContext = platformDisplay.sharingGLContext() ? static_cast<GLContextGLX*>(platformDisplay.sharingGLContext())->m_context.get() : nullptr;
    auto context = window ? createWindowContext(window, platformDisplay, glxSharingContext) : nullptr;
    if (!context)
        context = createPbufferContext(platformDisplay, glxSharingContext);
    if (!context)
        context = createPixmapContext(platformDisplay, glxSharingContext);

    return context;
}

std::unique_ptr<GLContextGLX> GLContextGLX::createSharingContext(PlatformDisplay& platformDisplay)
{
    auto context = createPbufferContext(platformDisplay);
    if (!context)
        context = createPixmapContext(platformDisplay);
    return context;
}

GLContextGLX::GLContextGLX(PlatformDisplay& display, XUniqueGLXContext&& context, GLNativeWindowType window)
    : GLContext(display)
    , m_x11Display(downcast<PlatformDisplayX11>(m_display).native())
    , m_context(WTFMove(context))
    , m_window(static_cast<Window>(window))
{
}

GLContextGLX::GLContextGLX(PlatformDisplay& display, XUniqueGLXContext&& context, XUniqueGLXPbuffer&& pbuffer)
    : GLContext(display)
    , m_x11Display(downcast<PlatformDisplayX11>(m_display).native())
    , m_context(WTFMove(context))
    , m_pbuffer(WTFMove(pbuffer))
{
}

GLContextGLX::GLContextGLX(PlatformDisplay& display, XUniqueGLXContext&& context, XUniquePixmap&& pixmap, XUniqueGLXPixmap&& glxPixmap)
    : GLContext(display)
    , m_x11Display(downcast<PlatformDisplayX11>(m_display).native())
    , m_context(WTFMove(context))
    , m_pixmap(WTFMove(pixmap))
    , m_glxPixmap(WTFMove(glxPixmap))
{
}

GLContextGLX::~GLContextGLX()
{
    if (m_cairoDevice)
        cairo_device_destroy(m_cairoDevice);

    if (m_context) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glXMakeCurrent(m_x11Display, None, None);
    }
}

bool GLContextGLX::canRenderToDefaultFramebuffer()
{
    return m_window;
}

IntSize GLContextGLX::defaultFrameBufferSize()
{
    if (!canRenderToDefaultFramebuffer() || !m_window)
        return IntSize();

    int x, y;
    Window rootWindow;
    unsigned int width, height, borderWidth, depth;
    if (!XGetGeometry(m_x11Display, m_window, &rootWindow, &x, &y, &width, &height, &borderWidth, &depth))
        return IntSize();

    return IntSize(width, height);
}

bool GLContextGLX::makeContextCurrent()
{
    ASSERT(m_context && (m_window || m_pbuffer || m_glxPixmap));

    GLContext::makeContextCurrent();
    if (glXGetCurrentContext() == m_context.get())
        return true;

    if (m_window)
        return glXMakeCurrent(m_x11Display, m_window, m_context.get());

    if (m_pbuffer)
        return glXMakeCurrent(m_x11Display, m_pbuffer.get(), m_context.get());

    return ::glXMakeCurrent(m_x11Display, m_glxPixmap.get(), m_context.get());
}

void GLContextGLX::swapBuffers()
{
    if (m_window)
        glXSwapBuffers(m_x11Display, m_window);
}

void GLContextGLX::waitNative()
{
    glXWaitX();
}

void GLContextGLX::swapInterval(int interval)
{
    if (!hasSGISwapControlExtension(m_x11Display))
        return;
    glXSwapIntervalSGI(interval);
}

cairo_device_t* GLContextGLX::cairoDevice()
{
    if (m_cairoDevice)
        return m_cairoDevice;

#if ENABLE(ACCELERATED_2D_CANVAS) && CAIRO_HAS_GLX_FUNCTIONS
    m_cairoDevice = cairo_glx_device_create(m_x11Display, m_context.get());
#endif

    return m_cairoDevice;
}

#if ENABLE(GRAPHICS_CONTEXT_3D)
PlatformGraphicsContext3D GLContextGLX::platformContext()
{
    return m_context.get();
}
#endif

} // namespace WebCore

#endif // USE(GLX)
