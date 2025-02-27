/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
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

#pragma once

#include "FloatSize.h"
#include "ImageOrientation.h"
#include "IntSize.h"
#include "TextFlags.h"
#include <wtf/Forward.h>

#if PLATFORM(IOS)
#include <wtf/RetainPtr.h>
typedef struct CGImage *CGImageRef;
#elif PLATFORM(MAC)
#include <wtf/RetainPtr.h>
OBJC_CLASS NSImage;
#elif PLATFORM(WIN)
typedef struct HBITMAP__* HBITMAP;
#elif PLATFORM(GTK)
#include "RefPtrCairo.h"
#endif

// We need to #define YOffset as it needs to be shared with WebKit
#define DragLabelBorderYOffset 2

namespace WebCore {

class Frame;
class Image;
class IntRect;
class Node;
class Range;
class URL;

#if PLATFORM(IOS)
typedef RetainPtr<CGImageRef> DragImageRef;
#elif PLATFORM(MAC)
typedef RetainPtr<NSImage> DragImageRef;
#elif PLATFORM(WIN)
typedef HBITMAP DragImageRef;
#elif PLATFORM(GTK)
typedef RefPtr<cairo_surface_t> DragImageRef;
#elif PLATFORM(EFL)
typedef void* DragImageRef;
#endif

#if PLATFORM(COCOA)
static const float SelectionDragImagePadding = 15;
#endif

IntSize dragImageSize(DragImageRef);

// These functions should be memory neutral, eg. if they return a newly allocated image,
// they should release the input image. As a corollary these methods don't guarantee
// the input image ref will still be valid after they have been called.
DragImageRef fitDragImageToMaxSize(DragImageRef, const IntSize& srcSize, const IntSize& dstSize);
DragImageRef scaleDragImage(DragImageRef, FloatSize scale);
DragImageRef platformAdjustDragImageForDeviceScaleFactor(DragImageRef, float deviceScaleFactor);
DragImageRef dissolveDragImageToFraction(DragImageRef, float delta);

DragImageRef createDragImageFromImage(Image*, ImageOrientationDescription);
DragImageRef createDragImageIconForCachedImageFilename(const String&);

WEBCORE_EXPORT DragImageRef createDragImageForNode(Frame&, Node&);
WEBCORE_EXPORT DragImageRef createDragImageForSelection(Frame&, bool forceBlackText = false);
WEBCORE_EXPORT DragImageRef createDragImageForRange(Frame&, Range&, bool forceBlackText = false);
DragImageRef createDragImageForImage(Frame&, Node&, IntRect& imageRect, IntRect& elementRect);
DragImageRef createDragImageForLink(URL&, const String& label, FontRenderingMode);
void deleteDragImage(DragImageRef);

class DragImage final {
public:
    DragImage();
    explicit DragImage(DragImageRef);
    DragImage(DragImage&&);
    ~DragImage();

    DragImage& operator=(DragImage&&);

    explicit operator bool() const { return !!m_dragImageRef; }
    DragImageRef get() const { return m_dragImageRef; }

private:
    DragImageRef m_dragImageRef;
};

}
