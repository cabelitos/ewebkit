/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2012 Company 100, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "CoordinatedGraphicsArgumentCoders.h"

#if USE(COORDINATED_GRAPHICS)
#include "WebCoordinatedSurface.h"
#include "WebCoreArgumentCoders.h"
#include <WebCore/Animation.h>
#include <WebCore/Color.h>
#include <WebCore/CoordinatedGraphicsState.h>
#include <WebCore/FilterOperations.h>
#include <WebCore/FloatPoint3D.h>
#include <WebCore/IdentityTransformOperation.h>
#include <WebCore/IntPoint.h>
#include <WebCore/Length.h>
#include <WebCore/Matrix3DTransformOperation.h>
#include <WebCore/MatrixTransformOperation.h>
#include <WebCore/PerspectiveTransformOperation.h>
#include <WebCore/RotateTransformOperation.h>
#include <WebCore/ScaleTransformOperation.h>
#include <WebCore/SkewTransformOperation.h>
#include <WebCore/SurfaceUpdateInfo.h>
#include <WebCore/TextureMapperAnimation.h>
#include <WebCore/TimingFunction.h>
#include <WebCore/TransformationMatrix.h>
#include <WebCore/TranslateTransformOperation.h>

#if USE(GRAPHICS_SURFACE)
#include <WebCore/GraphicsSurface.h>
#endif

using namespace WebCore;
using namespace WebKit;

namespace IPC {

void ArgumentCoder<WebCore::FilterOperations>::encode(Encoder& encoder, const WebCore::FilterOperations& filters)
{
    encoder << static_cast<uint32_t>(filters.size());
    for (size_t i = 0; i < filters.size(); ++i) {
        const FilterOperation& filter = *filters.at(i);
        FilterOperation::OperationType type = filter.type();
        encoder.encodeEnum(type);
        switch (type) {
        case FilterOperation::GRAYSCALE:
        case FilterOperation::SEPIA:
        case FilterOperation::SATURATE:
        case FilterOperation::HUE_ROTATE:
            encoder << static_cast<double>(downcast<BasicColorMatrixFilterOperation>(filter).amount());
            break;
        case FilterOperation::INVERT:
        case FilterOperation::BRIGHTNESS:
        case FilterOperation::CONTRAST:
        case FilterOperation::OPACITY:
            encoder << static_cast<double>(downcast<BasicComponentTransferFilterOperation>(filter).amount());
            break;
        case FilterOperation::BLUR:
            ArgumentCoder<Length>::encode(encoder, downcast<BlurFilterOperation>(filter).stdDeviation());
            break;
        case FilterOperation::DROP_SHADOW: {
            const DropShadowFilterOperation& shadow = downcast<DropShadowFilterOperation>(filter);
            ArgumentCoder<IntPoint>::encode(encoder, shadow.location());
            encoder << static_cast<int32_t>(shadow.stdDeviation());
            ArgumentCoder<Color>::encode(encoder, shadow.color());
            break;
        }
        case FilterOperation::REFERENCE:
        case FilterOperation::PASSTHROUGH:
        case FilterOperation::DEFAULT:
        case FilterOperation::NONE:
            break;
        }
    }
}

bool ArgumentCoder<WebCore::FilterOperations>::decode(Decoder& decoder, WebCore::FilterOperations& filters)
{
    uint32_t size;
    if (!decoder.decode(size))
        return false;

    Vector<RefPtr<FilterOperation> >& operations = filters.operations();

    for (size_t i = 0; i < size; ++i) {
        FilterOperation::OperationType type;
        RefPtr<FilterOperation> filter;
        if (!decoder.decodeEnum(type))
            return false;

        switch (type) {
        case FilterOperation::GRAYSCALE:
        case FilterOperation::SEPIA:
        case FilterOperation::SATURATE:
        case FilterOperation::HUE_ROTATE: {
            double value;
            if (!decoder.decode(value))
                return false;
            filter = BasicColorMatrixFilterOperation::create(value, type);
            break;
        }
        case FilterOperation::INVERT:
        case FilterOperation::BRIGHTNESS:
        case FilterOperation::CONTRAST:
        case FilterOperation::OPACITY: {
            double value;
            if (!decoder.decode(value))
                return false;
            filter = BasicComponentTransferFilterOperation::create(value, type);
            break;
        }
        case FilterOperation::BLUR: {
            Length length;
            if (!ArgumentCoder<Length>::decode(decoder, length))
                return false;
            filter = BlurFilterOperation::create(length);
            break;
        }
        case FilterOperation::DROP_SHADOW: {
            IntPoint location;
            int32_t stdDeviation;
            Color color;
            if (!ArgumentCoder<IntPoint>::decode(decoder, location))
                return false;
            if (!decoder.decode(stdDeviation))
                return false;
            if (!ArgumentCoder<Color>::decode(decoder, color))
                return false;
            filter = DropShadowFilterOperation::create(location, stdDeviation, color);
            break;
        }
        case FilterOperation::REFERENCE:
        case FilterOperation::PASSTHROUGH:
        case FilterOperation::DEFAULT:
        case FilterOperation::NONE:
            break;
        }

        if (filter)
            operations.append(filter);
    }

    return true;
}

void ArgumentCoder<TransformOperations>::encode(Encoder& encoder, const TransformOperations& transformOperations)
{
    encoder << static_cast<uint32_t>(transformOperations.size());
    for (const auto& operation : transformOperations.operations()) {
        encoder.encodeEnum(operation->type());

        switch (operation->type()) {
        case TransformOperation::SCALE_X:
        case TransformOperation::SCALE_Y:
        case TransformOperation::SCALE:
        case TransformOperation::SCALE_Z:
        case TransformOperation::SCALE_3D: {
            const auto& scaleOperation = downcast<ScaleTransformOperation>(*operation);
            encoder << scaleOperation.x();
            encoder << scaleOperation.y();
            encoder << scaleOperation.z();
            break;
        }
        case TransformOperation::TRANSLATE_X:
        case TransformOperation::TRANSLATE_Y:
        case TransformOperation::TRANSLATE:
        case TransformOperation::TRANSLATE_Z:
        case TransformOperation::TRANSLATE_3D: {
            const auto& translateOperation = downcast<TranslateTransformOperation>(*operation);
            ArgumentCoder<Length>::encode(encoder, translateOperation.x());
            ArgumentCoder<Length>::encode(encoder, translateOperation.y());
            ArgumentCoder<Length>::encode(encoder, translateOperation.z());
            break;
        }
        case TransformOperation::ROTATE:
        case TransformOperation::ROTATE_X:
        case TransformOperation::ROTATE_Y:
        case TransformOperation::ROTATE_3D: {
            const auto& rotateOperation = downcast<RotateTransformOperation>(*operation);
            encoder << rotateOperation.x();
            encoder << rotateOperation.y();
            encoder << rotateOperation.z();
            encoder << rotateOperation.angle();
            break;
        }
        case TransformOperation::SKEW_X:
        case TransformOperation::SKEW_Y:
        case TransformOperation::SKEW: {
            const auto& skewOperation = downcast<SkewTransformOperation>(*operation);
            encoder << skewOperation.angleX();
            encoder << skewOperation.angleY();
            break;
        }
        case TransformOperation::MATRIX:
            ArgumentCoder<TransformationMatrix>::encode(encoder, downcast<MatrixTransformOperation>(*operation).matrix());
            break;
        case TransformOperation::MATRIX_3D:
            ArgumentCoder<TransformationMatrix>::encode(encoder, downcast<Matrix3DTransformOperation>(*operation).matrix());
            break;
        case TransformOperation::PERSPECTIVE:
            ArgumentCoder<Length>::encode(encoder, downcast<PerspectiveTransformOperation>(*operation).perspective());
            break;
        case TransformOperation::IDENTITY:
            break;
        case TransformOperation::NONE:
            ASSERT_NOT_REACHED();
            break;
        }
    }
}

bool ArgumentCoder<TransformOperations>::decode(Decoder& decoder, TransformOperations& transformOperations)
{
    uint32_t operationsSize;
    if (!decoder.decode(operationsSize))
        return false;

    for (size_t i = 0; i < operationsSize; ++i) {
        TransformOperation::OperationType operationType;
        if (!decoder.decodeEnum(operationType))
            return false;

        switch (operationType) {
        case TransformOperation::SCALE_X:
        case TransformOperation::SCALE_Y:
        case TransformOperation::SCALE:
        case TransformOperation::SCALE_Z:
        case TransformOperation::SCALE_3D: {
            double x, y, z;
            if (!decoder.decode(x))
                return false;
            if (!decoder.decode(y))
                return false;
            if (!decoder.decode(z))
                return false;
            transformOperations.operations().append(ScaleTransformOperation::create(x, y, z, operationType));
            break;
        }
        case TransformOperation::TRANSLATE_X:
        case TransformOperation::TRANSLATE_Y:
        case TransformOperation::TRANSLATE:
        case TransformOperation::TRANSLATE_Z:
        case TransformOperation::TRANSLATE_3D: {
            Length x, y, z;
            if (!ArgumentCoder<Length>::decode(decoder, x))
                return false;
            if (!ArgumentCoder<Length>::decode(decoder, y))
                return false;
            if (!ArgumentCoder<Length>::decode(decoder, z))
                return false;
            transformOperations.operations().append(TranslateTransformOperation::create(x, y, z, operationType));
            break;
        }
        case TransformOperation::ROTATE:
        case TransformOperation::ROTATE_X:
        case TransformOperation::ROTATE_Y:
        case TransformOperation::ROTATE_3D: {
            double x, y, z, angle;
            if (!decoder.decode(x))
                return false;
            if (!decoder.decode(y))
                return false;
            if (!decoder.decode(z))
                return false;
            if (!decoder.decode(angle))
                return false;
            transformOperations.operations().append(RotateTransformOperation::create(x, y, z, angle, operationType));
            break;
        }
        case TransformOperation::SKEW_X:
        case TransformOperation::SKEW_Y:
        case TransformOperation::SKEW: {
            double angleX, angleY;
            if (!decoder.decode(angleX))
                return false;
            if (!decoder.decode(angleY))
                return false;
            transformOperations.operations().append(SkewTransformOperation::create(angleX, angleY, operationType));
            break;
        }
        case TransformOperation::MATRIX: {
            TransformationMatrix matrix;
            if (!ArgumentCoder<TransformationMatrix>::decode(decoder, matrix))
                return false;
            transformOperations.operations().append(MatrixTransformOperation::create(matrix));
            break;
        }
        case TransformOperation::MATRIX_3D: {
            TransformationMatrix matrix;
            if (!ArgumentCoder<TransformationMatrix>::decode(decoder, matrix))
                return false;
            transformOperations.operations().append(Matrix3DTransformOperation::create(matrix));
            break;
        }
        case TransformOperation::PERSPECTIVE: {
            Length perspective;
            if (!ArgumentCoder<Length>::decode(decoder, perspective))
                return false;
            transformOperations.operations().append(PerspectiveTransformOperation::create(perspective));
            break;
        }
        case TransformOperation::IDENTITY:
            transformOperations.operations().append(IdentityTransformOperation::create());
            break;
        case TransformOperation::NONE:
            ASSERT_NOT_REACHED();
            break;
        }
    }
    return true;
}

static void encodeTimingFunction(Encoder& encoder, const TimingFunction* timingFunction)
{
    if (!timingFunction) {
        encoder.encodeEnum(TimingFunction::TimingFunctionType(-1));
        return;
    }

    TimingFunction::TimingFunctionType type = timingFunction ? timingFunction->type() : TimingFunction::LinearFunction;
    encoder.encodeEnum(type);
    switch (type) {
    case TimingFunction::LinearFunction:
        break;
    case TimingFunction::CubicBezierFunction: {
        const CubicBezierTimingFunction* cubic = static_cast<const CubicBezierTimingFunction*>(timingFunction);
        CubicBezierTimingFunction::TimingFunctionPreset bezierPreset = cubic->timingFunctionPreset();
        encoder.encodeEnum(bezierPreset);
        if (bezierPreset == CubicBezierTimingFunction::Custom) {
            encoder << cubic->x1();
            encoder << cubic->y1();
            encoder << cubic->x2();
            encoder << cubic->y2();
        }
        break;
    }
    case TimingFunction::StepsFunction: {
        const StepsTimingFunction* steps = static_cast<const StepsTimingFunction*>(timingFunction);
        encoder << static_cast<uint32_t>(steps->numberOfSteps());
        encoder << steps->stepAtStart();
        break;
    }
    case TimingFunction::SpringFunction: {
        const SpringTimingFunction* spring = static_cast<const SpringTimingFunction*>(timingFunction);
        encoder << spring->mass();
        encoder << spring->stiffness();
        encoder << spring->damping();
        encoder << spring->initialVelocity();
        break;
    }
    }
}

bool decodeTimingFunction(Decoder& decoder, RefPtr<TimingFunction>& timingFunction)
{
    TimingFunction::TimingFunctionType type;
    if (!decoder.decodeEnum(type))
        return false;

    if (type == TimingFunction::TimingFunctionType(-1))
        return true;

    switch (type) {
    case TimingFunction::LinearFunction:
        timingFunction = LinearTimingFunction::create();
        return true;
    case TimingFunction::CubicBezierFunction: {
        double x1, y1, x2, y2;
        CubicBezierTimingFunction::TimingFunctionPreset bezierPreset;
        if (!decoder.decodeEnum(bezierPreset))
            return false;
        if (bezierPreset != CubicBezierTimingFunction::Custom) {
            timingFunction = CubicBezierTimingFunction::create(bezierPreset);
            return true;
        }
        if (!decoder.decode(x1))
            return false;
        if (!decoder.decode(y1))
            return false;
        if (!decoder.decode(x2))
            return false;
        if (!decoder.decode(y2))
            return false;

        timingFunction = CubicBezierTimingFunction::create(x1, y1, x2, y2);
        return true;
    }
    case TimingFunction::StepsFunction: {
        uint32_t numberOfSteps;
        bool stepAtStart;
        if (!decoder.decode(numberOfSteps))
            return false;
        if (!decoder.decode(stepAtStart))
            return false;

        timingFunction = StepsTimingFunction::create(numberOfSteps, stepAtStart);
        return true;
    }
    case TimingFunction::SpringFunction: {
        double mass;
        if (!decoder.decode(mass))
            return false;
        double stiffness;
        if (!decoder.decode(stiffness))
            return false;
        double damping;
        if (!decoder.decode(damping))
            return false;
        double initialVelocity;
        if (!decoder.decode(initialVelocity))
            return false;

        timingFunction = SpringTimingFunction::create(mass, stiffness, damping, initialVelocity);
        return true;
    }
    }

    return false;
}

void ArgumentCoder<TextureMapperAnimation>::encode(Encoder& encoder, const TextureMapperAnimation& animation)
{
    encoder << animation.name();
    encoder << animation.boxSize();
    encoder.encodeEnum(animation.state());
    encoder << animation.startTime();
    encoder << animation.pauseTime();
    encoder << animation.listsMatch();

    RefPtr<Animation> animationObject = animation.animation();
    encoder.encodeEnum(animationObject->direction());
    encoder << static_cast<uint32_t>(animationObject->fillMode());
    encoder << animationObject->duration();
    encoder << animationObject->iterationCount();
    encodeTimingFunction(encoder, animationObject->timingFunction());

    const KeyframeValueList& keyframes = animation.keyframes();
    encoder.encodeEnum(keyframes.property());
    encoder << static_cast<uint32_t>(keyframes.size());
    for (size_t i = 0; i < keyframes.size(); ++i) {
        const AnimationValue& value = keyframes.at(i);
        encoder << value.keyTime();
        encodeTimingFunction(encoder, value.timingFunction());
        switch (keyframes.property()) {
        case AnimatedPropertyOpacity:
            encoder << static_cast<const FloatAnimationValue&>(value).value();
            break;
        case AnimatedPropertyTransform:
            encoder << static_cast<const TransformAnimationValue&>(value).value();
            break;
        case AnimatedPropertyFilter:
            encoder << static_cast<const FilterAnimationValue&>(value).value();
            break;
        default:
            break;
        }
    }
}

bool ArgumentCoder<TextureMapperAnimation>::decode(Decoder& decoder, TextureMapperAnimation& animation)
{
    String name;
    FloatSize boxSize;
    TextureMapperAnimation::AnimationState state;
    double startTime;
    double pauseTime;
    bool listsMatch;

    Animation::AnimationDirection direction;
    unsigned fillMode;
    double duration;
    double iterationCount;
    RefPtr<TimingFunction> timingFunction;
    RefPtr<Animation> animationObject;

    if (!decoder.decode(name))
        return false;
    if (!decoder.decode(boxSize))
        return false;
    if (!decoder.decodeEnum(state))
        return false;
    if (!decoder.decode(startTime))
        return false;
    if (!decoder.decode(pauseTime))
        return false;
    if (!decoder.decode(listsMatch))
        return false;
    if (!decoder.decodeEnum(direction))
        return false;
    if (!decoder.decode(fillMode))
        return false;
    if (!decoder.decode(duration))
        return false;
    if (!decoder.decode(iterationCount))
        return false;
    if (!decodeTimingFunction(decoder, timingFunction))
        return false;

    animationObject = Animation::create();
    animationObject->setDirection(direction);
    animationObject->setFillMode(fillMode);
    animationObject->setDuration(duration);
    animationObject->setIterationCount(iterationCount);
    if (timingFunction)
        animationObject->setTimingFunction(WTFMove(timingFunction));

    AnimatedPropertyID property;
    if (!decoder.decodeEnum(property))
        return false;
    KeyframeValueList keyframes(property);
    unsigned keyframesSize;
    if (!decoder.decode(keyframesSize))
        return false;
    for (unsigned i = 0; i < keyframesSize; ++i) {
        double keyTime;
        RefPtr<TimingFunction> timingFunction;
        if (!decoder.decode(keyTime))
            return false;
        if (!decodeTimingFunction(decoder, timingFunction))
            return false;

        switch (property) {
        case AnimatedPropertyOpacity: {
            float value;
            if (!decoder.decode(value))
                return false;
            keyframes.insert(std::make_unique<FloatAnimationValue>(keyTime, value, timingFunction.get()));
            break;
        }
        case AnimatedPropertyTransform: {
            TransformOperations transform;
            if (!decoder.decode(transform))
                return false;
            keyframes.insert(std::make_unique<TransformAnimationValue>(keyTime, transform, timingFunction.get()));
            break;
        }
        case AnimatedPropertyFilter: {
            FilterOperations filter;
            if (!decoder.decode(filter))
                return false;
            keyframes.insert(std::make_unique<FilterAnimationValue>(keyTime, filter, timingFunction.get()));
            break;
        }
        default:
            break;
        }
    }

    animation = TextureMapperAnimation(name, keyframes, boxSize, *animationObject, listsMatch, startTime, pauseTime, state);
    return true;
}

void ArgumentCoder<TextureMapperAnimations>::encode(Encoder& encoder, const TextureMapperAnimations& animations)
{
    encoder << animations.animations();
}

bool ArgumentCoder<TextureMapperAnimations>::decode(Decoder& decoder, TextureMapperAnimations& animations)
{
    return decoder.decode(animations.animations());
}

#if USE(GRAPHICS_SURFACE)
void ArgumentCoder<WebCore::GraphicsSurfaceToken>::encode(Encoder& encoder, const WebCore::GraphicsSurfaceToken& token)
{
#if OS(DARWIN)
    encoder << Attachment(token.frontBufferHandle, MACH_MSG_TYPE_MOVE_SEND);
    encoder << Attachment(token.backBufferHandle, MACH_MSG_TYPE_MOVE_SEND);
#elif OS(LINUX)
    encoder << token.frontBufferHandle;
#endif
}

bool ArgumentCoder<WebCore::GraphicsSurfaceToken>::decode(Decoder& decoder, WebCore::GraphicsSurfaceToken& token)
{
#if OS(DARWIN)
    Attachment frontAttachment, backAttachment;
    if (!decoder.decode(frontAttachment))
        return false;
    if (!decoder.decode(backAttachment))
        return false;

    token = GraphicsSurfaceToken(frontAttachment.port(), backAttachment.port());
#elif OS(LINUX)
    if (!decoder.decode(token.frontBufferHandle))
        return false;
#endif
    return true;
}
#endif

void ArgumentCoder<SurfaceUpdateInfo>::encode(Encoder& encoder, const SurfaceUpdateInfo& surfaceUpdateInfo)
{
    SimpleArgumentCoder<SurfaceUpdateInfo>::encode(encoder, surfaceUpdateInfo);
}

bool ArgumentCoder<SurfaceUpdateInfo>::decode(Decoder& decoder, SurfaceUpdateInfo& surfaceUpdateInfo)
{
    return SimpleArgumentCoder<SurfaceUpdateInfo>::decode(decoder, surfaceUpdateInfo);
}

void ArgumentCoder<CoordinatedGraphicsLayerState>::encode(Encoder& encoder, const CoordinatedGraphicsLayerState& state)
{
    encoder << state.changeMask;

    if (state.flagsChanged)
        encoder << state.flags;

    if (state.positionChanged)
        encoder << state.pos;

    if (state.anchorPointChanged)
        encoder << state.anchorPoint;

    if (state.sizeChanged)
        encoder << state.size;

    if (state.transformChanged)
        encoder << state.transform;

    if (state.childrenTransformChanged)
        encoder << state.childrenTransform;

    if (state.contentsRectChanged)
        encoder << state.contentsRect;

    if (state.contentsTilingChanged) {
        encoder << state.contentsTileSize;
        encoder << state.contentsTilePhase;
    }

    if (state.opacityChanged)
        encoder << state.opacity;

    if (state.solidColorChanged)
        encoder << state.solidColor;

    if (state.debugBorderColorChanged)
        encoder << state.debugBorderColor;

    if (state.debugBorderWidthChanged)
        encoder << state.debugBorderWidth;

    if (state.filtersChanged)
        encoder << state.filters;

    if (state.animationsChanged)
        encoder << state.animations;

    if (state.childrenChanged)
        encoder << state.children;

    encoder << state.tilesToCreate;
    encoder << state.tilesToRemove;

    if (state.replicaChanged)
        encoder << state.replica;

    if (state.maskChanged)
        encoder << state.mask;

    if (state.imageChanged)
        encoder << state.imageID;

    if (state.repaintCountChanged)
        encoder << state.repaintCount;

    encoder << state.tilesToUpdate;

#if USE(GRAPHICS_SURFACE)
    if (state.platformLayerChanged) {
        encoder << state.platformLayerSize;
        encoder << state.platformLayerToken;
        encoder << state.platformLayerFrontBuffer;
        encoder << state.platformLayerSurfaceFlags;
    }
#endif

    if (state.committedScrollOffsetChanged)
        encoder << state.committedScrollOffset;
}

bool ArgumentCoder<CoordinatedGraphicsLayerState>::decode(Decoder& decoder, CoordinatedGraphicsLayerState& state)
{
    if (!decoder.decode(state.changeMask))
        return false;

    if (state.flagsChanged && !decoder.decode(state.flags))
        return false;

    if (state.positionChanged && !decoder.decode(state.pos))
        return false;

    if (state.anchorPointChanged && !decoder.decode(state.anchorPoint))
        return false;

    if (state.sizeChanged && !decoder.decode(state.size))
        return false;

    if (state.transformChanged && !decoder.decode(state.transform))
        return false;

    if (state.childrenTransformChanged && !decoder.decode(state.childrenTransform))
        return false;

    if (state.contentsRectChanged && !decoder.decode(state.contentsRect))
        return false;

    if (state.contentsTilingChanged) {
        if (!decoder.decode(state.contentsTileSize))
            return false;
        if (!decoder.decode(state.contentsTilePhase))
            return false;
    }

    if (state.opacityChanged && !decoder.decode(state.opacity))
        return false;

    if (state.solidColorChanged && !decoder.decode(state.solidColor))
        return false;

    if (state.debugBorderColorChanged && !decoder.decode(state.debugBorderColor))
        return false;

    if (state.debugBorderWidthChanged && !decoder.decode(state.debugBorderWidth))
        return false;

    if (state.filtersChanged && !decoder.decode(state.filters))
        return false;

    if (state.animationsChanged && !decoder.decode(state.animations))
        return false;

    if (state.childrenChanged && !decoder.decode(state.children))
        return false;

    if (!decoder.decode(state.tilesToCreate))
        return false;

    if (!decoder.decode(state.tilesToRemove))
        return false;

    if (state.replicaChanged && !decoder.decode(state.replica))
        return false;

    if (state.maskChanged && !decoder.decode(state.mask))
        return false;

    if (state.imageChanged && !decoder.decode(state.imageID))
        return false;

    if (state.repaintCountChanged && !decoder.decode(state.repaintCount))
        return false;

    if (!decoder.decode(state.tilesToUpdate))
        return false;

#if USE(GRAPHICS_SURFACE)
    if (state.platformLayerChanged) {
        if (!decoder.decode(state.platformLayerSize))
            return false;

        if (!decoder.decode(state.platformLayerToken))
            return false;

        if (!decoder.decode(state.platformLayerFrontBuffer))
            return false;

        if (!decoder.decode(state.platformLayerSurfaceFlags))
            return false;
    }
#endif

    if (state.committedScrollOffsetChanged && !decoder.decode(state.committedScrollOffset))
        return false;

    return true;
}

void ArgumentCoder<TileUpdateInfo>::encode(Encoder& encoder, const TileUpdateInfo& updateInfo)
{
    SimpleArgumentCoder<TileUpdateInfo>::encode(encoder, updateInfo);
}

bool ArgumentCoder<TileUpdateInfo>::decode(Decoder& decoder, TileUpdateInfo& updateInfo)
{
    return SimpleArgumentCoder<TileUpdateInfo>::decode(decoder, updateInfo);
}

void ArgumentCoder<TileCreationInfo>::encode(Encoder& encoder, const TileCreationInfo& updateInfo)
{
    SimpleArgumentCoder<TileCreationInfo>::encode(encoder, updateInfo);
}

bool ArgumentCoder<TileCreationInfo>::decode(Decoder& decoder, TileCreationInfo& updateInfo)
{
    return SimpleArgumentCoder<TileCreationInfo>::decode(decoder, updateInfo);
}

static void encodeCoordinatedSurface(Encoder& encoder, const RefPtr<CoordinatedSurface>& surface)
{
    bool isValidSurface = false;
    if (!surface) {
        encoder << isValidSurface;
        return;
    }

    WebCoordinatedSurface* webCoordinatedSurface = static_cast<WebCoordinatedSurface*>(surface.get());
    WebCoordinatedSurface::Handle handle;
    if (webCoordinatedSurface->createHandle(handle))
        isValidSurface = true;

    encoder << isValidSurface;

    if (isValidSurface)
        encoder << handle;
}

static bool decodeCoordinatedSurface(Decoder& decoder, RefPtr<CoordinatedSurface>& surface)
{
    bool isValidSurface;
    if (!decoder.decode(isValidSurface))
        return false;

    if (!isValidSurface)
        return true;

    WebCoordinatedSurface::Handle handle;
    if (!decoder.decode(handle))
        return false;

    surface = WebCoordinatedSurface::create(handle);
    return true;
}

void ArgumentCoder<CoordinatedGraphicsState>::encode(Encoder& encoder, const CoordinatedGraphicsState& state)
{
    encoder << state.rootCompositingLayer;
    encoder << state.scrollPosition;
    encoder << state.contentsSize;
    encoder << state.coveredRect;

    encoder << state.layersToCreate;
    encoder << state.layersToUpdate;
    encoder << state.layersToRemove;

    encoder << state.imagesToCreate;
    encoder << state.imagesToRemove;

    // We need to encode WebCoordinatedSurface::Handle right after it's creation.
    // That's why we cannot use simple std::pair encoder.
    encoder << static_cast<uint64_t>(state.imagesToUpdate.size());
    for (size_t i = 0; i < state.imagesToUpdate.size(); ++i) {
        encoder << state.imagesToUpdate[i].first;
        encodeCoordinatedSurface(encoder, state.imagesToUpdate[i].second);
    }
    encoder << state.imagesToClear;

    encoder << static_cast<uint64_t>(state.updateAtlasesToCreate.size());
    for (size_t i = 0; i < state.updateAtlasesToCreate.size(); ++i) {
        encoder << state.updateAtlasesToCreate[i].first;
        encodeCoordinatedSurface(encoder, state.updateAtlasesToCreate[i].second);
    }
    encoder << state.updateAtlasesToRemove;
}

bool ArgumentCoder<CoordinatedGraphicsState>::decode(Decoder& decoder, CoordinatedGraphicsState& state)
{
    if (!decoder.decode(state.rootCompositingLayer))
        return false;

    if (!decoder.decode(state.scrollPosition))
        return false;

    if (!decoder.decode(state.contentsSize))
        return false;

    if (!decoder.decode(state.coveredRect))
        return false;

    if (!decoder.decode(state.layersToCreate))
        return false;

    if (!decoder.decode(state.layersToUpdate))
        return false;

    if (!decoder.decode(state.layersToRemove))
        return false;

    if (!decoder.decode(state.imagesToCreate))
        return false;

    if (!decoder.decode(state.imagesToRemove))
        return false;

    uint64_t sizeOfImagesToUpdate;
    if (!decoder.decode(sizeOfImagesToUpdate))
        return false;

    for (uint64_t i = 0; i < sizeOfImagesToUpdate; ++i) {
        CoordinatedImageBackingID imageID;
        if (!decoder.decode(imageID))
            return false;

        RefPtr<CoordinatedSurface> surface;
        if (!decodeCoordinatedSurface(decoder, surface))
            return false;

        state.imagesToUpdate.append(std::make_pair(imageID, surface.release()));
    }

    if (!decoder.decode(state.imagesToClear))
        return false;

    uint64_t sizeOfUpdateAtlasesToCreate;
    if (!decoder.decode(sizeOfUpdateAtlasesToCreate))
        return false;

    for (uint64_t i = 0; i < sizeOfUpdateAtlasesToCreate; ++i) {
        uint32_t atlasID;
        if (!decoder.decode(atlasID))
            return false;

        RefPtr<CoordinatedSurface> surface;
        if (!decodeCoordinatedSurface(decoder, surface))
            return false;

        state.updateAtlasesToCreate.append(std::make_pair(atlasID, surface.release()));
    }

    if (!decoder.decode(state.updateAtlasesToRemove))
        return false;

    return true;
}

} // namespace IPC

#endif // USE(COORDINATED_GRAPHICS)
