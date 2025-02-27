/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "AnimationList.h"
#include "BorderValue.h"
#include "CSSLineBoxContainValue.h"
#include "CSSPrimitiveValue.h"
#include "CSSPropertyNames.h"
#include "Color.h"
#include "CounterDirectives.h"
#include "DataRef.h"
#include "FontDescription.h"
#include "GraphicsTypes.h"
#include "Length.h"
#include "LengthBox.h"
#include "LengthPoint.h"
#include "LengthSize.h"
#include "LineClampValue.h"
#include "NinePieceImage.h"
#include "Pagination.h"
#include "RenderStyleConstants.h"
#include "RoundedRect.h"
#include "SVGRenderStyle.h"
#include "ShadowData.h"
#include "ShapeValue.h"
#include "StyleBackgroundData.h"
#include "StyleBoxData.h"
#include "StyleDeprecatedFlexibleBoxData.h"
#include "StyleFilterData.h"
#include "StyleFlexibleBoxData.h"
#include "StyleMarqueeData.h"
#include "StyleMultiColData.h"
#include "StyleRareInheritedData.h"
#include "StyleRareNonInheritedData.h"
#include "StyleReflection.h"
#include "StyleSurroundData.h"
#include "StyleTransformData.h"
#include "StyleVisualData.h"
#include "TextFlags.h"
#include "ThemeTypes.h"
#include "TransformOperations.h"
#include "UnicodeBidi.h"
#include <memory>
#include <wtf/Forward.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

#if ENABLE(CSS_GRID_LAYOUT)
#include "StyleGridData.h"
#include "StyleGridItemData.h"
#endif

#if ENABLE(DASHBOARD_SUPPORT)
#include "StyleDashboardRegion.h"
#endif

#if ENABLE(TEXT_AUTOSIZING)
#include "TextSizeAdjustment.h"
#endif

#define SET_VAR(group, variable, value) do { \
        if (!compareEqual(group->variable, value)) \
            group.access().variable = value; \
    } while (0)

#define SET_NESTED_VAR(group, parentVariable, variable, value) do { \
        if (!compareEqual(group->parentVariable->variable, value)) \
            group.access().parentVariable.access().variable = value; \
    } while (0)

#define SET_BORDERVALUE_COLOR(group, variable, value) do { \
        if (!compareEqual(group->variable.color(), value)) \
            group.access().variable.setColor(value); \
    } while (0)

namespace WebCore {

class BorderData;
class ContentData;
class CounterContent;
class CursorList;
class FilterOperations;
class FontCascade;
class FontMetrics;
class IntRect;
class Pair;
class ShadowData;
class StyleImage;
class StyleInheritedData;
class StyleResolver;
class StyleScrollSnapArea;
class StyleScrollSnapPort;
class TransformationMatrix;

struct ScrollSnapAlign;
struct ScrollSnapType;

using PseudoStyleCache = Vector<std::unique_ptr<RenderStyle>, 4>;

template<typename T, typename U> inline bool compareEqual(const T& t, const U& u) { return t == static_cast<const T&>(u); }

class RenderStyle {
    WTF_MAKE_FAST_ALLOCATED;

private:
    enum CloneTag { Clone };
    enum CreateDefaultStyleTag { CreateDefaultStyle };

public:
    RenderStyle(RenderStyle&&) = default;
    RenderStyle& operator=(RenderStyle&&) = default;
    ~RenderStyle();

    RenderStyle replace(RenderStyle&&) WARN_UNUSED_RETURN;

    explicit RenderStyle(CreateDefaultStyleTag);
    RenderStyle(const RenderStyle&, CloneTag);

    static RenderStyle& defaultStyle();

    static RenderStyle create();
    static std::unique_ptr<RenderStyle> createPtr();

    static RenderStyle clone(const RenderStyle&);
    static std::unique_ptr<RenderStyle> clonePtr(const RenderStyle&);

    static RenderStyle createAnonymousStyleWithDisplay(const RenderStyle& parentStyle, EDisplay);
    static RenderStyle createStyleInheritingFromPseudoStyle(const RenderStyle& pseudoStyle);

#if !ASSERT_DISABLED || ENABLE(SECURITY_ASSERTIONS)
    bool deletionHasBegun() const { return m_deletionHasBegun; }
#endif

    bool operator==(const RenderStyle&) const;
    bool operator!=(const RenderStyle& other) const { return !(*this == other); }

    void inheritFrom(const RenderStyle& inheritParent);
    void copyNonInheritedFrom(const RenderStyle&);

    ContentPosition resolvedJustifyContentPosition(const StyleContentAlignmentData& normalValueBehavior) const;
    ContentDistributionType resolvedJustifyContentDistribution(const StyleContentAlignmentData& normalValueBehavior) const;
    ContentPosition resolvedAlignContentPosition(const StyleContentAlignmentData& normalValueBehavior) const;
    ContentDistributionType resolvedAlignContentDistribution(const StyleContentAlignmentData& normalValueBehavior) const;
    StyleSelfAlignmentData resolvedAlignItems(ItemPosition normalValueBehaviour) const;
    StyleSelfAlignmentData resolvedAlignSelf(const RenderStyle& parentStyle, ItemPosition normalValueBehaviour) const;
    StyleSelfAlignmentData resolvedJustifyItems(ItemPosition normalValueBehaviour) const;
    StyleSelfAlignmentData resolvedJustifySelf(const RenderStyle& parentStyle, ItemPosition normalValueBehaviour) const;

    PseudoId styleType() const { return m_nonInheritedFlags.styleType(); }
    void setStyleType(PseudoId styleType) { m_nonInheritedFlags.setStyleType(styleType); }

    RenderStyle* getCachedPseudoStyle(PseudoId) const;
    RenderStyle* addCachedPseudoStyle(std::unique_ptr<RenderStyle>);
    void removeCachedPseudoStyle(PseudoId);

    const PseudoStyleCache* cachedPseudoStyles() const { return m_cachedPseudoStyles.get(); }

    const CustomPropertyValueMap& customProperties() const { return m_rareInheritedData->customProperties->values; }
    void setCustomPropertyValue(const AtomicString& name, Ref<CSSCustomPropertyValue>&& value) { return m_rareInheritedData.access().customProperties.access().setCustomPropertyValue(name, WTFMove(value)); }

    void setHasViewportUnits(bool hasViewportUnits = true) { m_nonInheritedFlags.setHasViewportUnits(hasViewportUnits); }
    bool hasViewportUnits() const { return m_nonInheritedFlags.hasViewportUnits(); }

    bool affectedByHover() const { return m_nonInheritedFlags.affectedByHover(); }
    bool affectedByActive() const { return m_nonInheritedFlags.affectedByActive(); }
    bool affectedByDrag() const { return m_nonInheritedFlags.affectedByDrag(); }

    void setAffectedByHover() { m_nonInheritedFlags.setAffectedByHover(true); }
    void setAffectedByActive() { m_nonInheritedFlags.setAffectedByActive(true); }
    void setAffectedByDrag() { m_nonInheritedFlags.setAffectedByDrag(true); }

    void setColumnStylesFromPaginationMode(const Pagination::Mode&);
    
    bool isFloating() const { return m_nonInheritedFlags.isFloating(); }
    bool hasMargin() const { return !m_surroundData->margin.isZero(); }
    bool hasBorder() const { return m_surroundData->border.hasBorder(); }
    bool hasBorderFill() const { return m_surroundData->border.hasFill(); }
    bool hasVisibleBorderDecoration() const { return hasVisibleBorder() || hasBorderFill(); }
    bool hasVisibleBorder() const { return m_surroundData->border.hasVisibleBorder(); }
    bool hasPadding() const { return !m_surroundData->padding.isZero(); }
    bool hasOffset() const { return !m_surroundData->offset.isZero(); }
    bool hasMarginBeforeQuirk() const { return marginBefore().hasQuirk(); }
    bool hasMarginAfterQuirk() const { return marginAfter().hasQuirk(); }

    bool hasBackgroundImage() const { return m_backgroundData->background.hasImage(); }
    bool hasFixedBackgroundImage() const { return m_backgroundData->background.hasFixedImage(); }

    bool hasEntirelyFixedBackground() const;

    bool hasAppearance() const { return appearance() != NoControlPart; }

    bool hasBackground() const;
    
    LayoutBoxExtent imageOutsets(const NinePieceImage&) const;
    bool hasBorderImageOutsets() const { return borderImage().hasImage() && !borderImage().outset().isZero(); }
    LayoutBoxExtent borderImageOutsets() const { return imageOutsets(borderImage()); }

    LayoutBoxExtent maskBoxImageOutsets() const { return imageOutsets(maskBoxImage()); }

    bool hasFilterOutsets() const { return hasFilter() && filter().hasOutsets(); }
    FilterOutsets filterOutsets() const { return hasFilter() ? filter().outsets() : FilterOutsets(); }

    Order rtlOrdering() const { return static_cast<Order>(m_inheritedFlags.rtlOrdering); }
    void setRTLOrdering(Order ordering) { m_inheritedFlags.rtlOrdering = ordering; }

    bool isStyleAvailable() const;

    bool hasAnyPublicPseudoStyles() const;
    bool hasPseudoStyle(PseudoId) const;
    void setHasPseudoStyle(PseudoId);
    void setHasPseudoStyles(PseudoIdSet);
    bool hasUniquePseudoStyle() const;

    // attribute getter methods

    EDisplay display() const { return m_nonInheritedFlags.effectiveDisplay(); }

    const Length& left() const { return m_surroundData->offset.left(); }
    const Length& right() const { return m_surroundData->offset.right(); }
    const Length& top() const { return m_surroundData->offset.top(); }
    const Length& bottom() const { return m_surroundData->offset.bottom(); }

    // Accessors for positioned object edges that take into account writing mode.
    const Length& logicalLeft() const { return m_surroundData->offset.start(writingMode()); }
    const Length& logicalRight() const { return m_surroundData->offset.end(writingMode()); }
    const Length& logicalTop() const { return m_surroundData->offset.before(writingMode()); }
    const Length& logicalBottom() const { return m_surroundData->offset.after(writingMode()); }

    // Whether or not a positioned element requires normal flow x/y to be computed  to determine its position.
    bool hasStaticInlinePosition(bool horizontal) const { return horizontal ? hasAutoLeftAndRight() : hasAutoTopAndBottom(); }
    bool hasStaticBlockPosition(bool horizontal) const { return horizontal ? hasAutoTopAndBottom() : hasAutoLeftAndRight(); }

    EPosition position() const { return m_nonInheritedFlags.position(); }
    bool hasOutOfFlowPosition() const { return position() == AbsolutePosition || position() == FixedPosition; }
    bool hasInFlowPosition() const { return position() == RelativePosition || position() == StickyPosition; }
    bool hasViewportConstrainedPosition() const { return position() == FixedPosition || position() == StickyPosition; }
    EFloat floating() const { return m_nonInheritedFlags.floating(); }

    const Length& width() const { return m_boxData->width(); }
    const Length& height() const { return m_boxData->height(); }
    const Length& minWidth() const { return m_boxData->minWidth(); }
    const Length& maxWidth() const { return m_boxData->maxWidth(); }
    const Length& minHeight() const { return m_boxData->minHeight(); }
    const Length& maxHeight() const { return m_boxData->maxHeight(); }
    
    const Length& logicalWidth() const { return isHorizontalWritingMode() ? width() : height(); }
    const Length& logicalHeight() const { return isHorizontalWritingMode() ? height() : width(); }
    const Length& logicalMinWidth() const { return isHorizontalWritingMode() ? minWidth() : minHeight(); }
    const Length& logicalMaxWidth() const { return isHorizontalWritingMode() ? maxWidth() : maxHeight(); }
    const Length& logicalMinHeight() const { return isHorizontalWritingMode() ? minHeight() : minWidth(); }
    const Length& logicalMaxHeight() const { return isHorizontalWritingMode() ? maxHeight() : maxWidth(); }

    const BorderData& border() const { return m_surroundData->border; }
    const BorderValue& borderLeft() const { return m_surroundData->border.left(); }
    const BorderValue& borderRight() const { return m_surroundData->border.right(); }
    const BorderValue& borderTop() const { return m_surroundData->border.top(); }
    const BorderValue& borderBottom() const { return m_surroundData->border.bottom(); }

    const BorderValue& borderBefore() const;
    const BorderValue& borderAfter() const;
    const BorderValue& borderStart() const;
    const BorderValue& borderEnd() const;

    const NinePieceImage& borderImage() const { return m_surroundData->border.image(); }
    StyleImage* borderImageSource() const { return m_surroundData->border.image().image(); }
    const LengthBox& borderImageSlices() const { return m_surroundData->border.image().imageSlices(); }
    const LengthBox& borderImageWidth() const { return m_surroundData->border.image().borderSlices(); }
    const LengthBox& borderImageOutset() const { return m_surroundData->border.image().outset(); }

    const LengthSize& borderTopLeftRadius() const { return m_surroundData->border.topLeft(); }
    const LengthSize& borderTopRightRadius() const { return m_surroundData->border.topRight(); }
    const LengthSize& borderBottomLeftRadius() const { return m_surroundData->border.bottomLeft(); }
    const LengthSize& borderBottomRightRadius() const { return m_surroundData->border.bottomRight(); }
    bool hasBorderRadius() const { return m_surroundData->border.hasBorderRadius(); }

    float borderLeftWidth() const { return m_surroundData->border.borderLeftWidth(); }
    EBorderStyle borderLeftStyle() const { return m_surroundData->border.left().style(); }
    bool borderLeftIsTransparent() const { return m_surroundData->border.left().isTransparent(); }
    float borderRightWidth() const { return m_surroundData->border.borderRightWidth(); }
    EBorderStyle borderRightStyle() const { return m_surroundData->border.right().style(); }
    bool borderRightIsTransparent() const { return m_surroundData->border.right().isTransparent(); }
    float borderTopWidth() const { return m_surroundData->border.borderTopWidth(); }
    EBorderStyle borderTopStyle() const { return m_surroundData->border.top().style(); }
    bool borderTopIsTransparent() const { return m_surroundData->border.top().isTransparent(); }
    float borderBottomWidth() const { return m_surroundData->border.borderBottomWidth(); }
    EBorderStyle borderBottomStyle() const { return m_surroundData->border.bottom().style(); }
    bool borderBottomIsTransparent() const { return m_surroundData->border.bottom().isTransparent(); }
    FloatBoxExtent borderWidth() const { return m_surroundData->border.borderWidth(); }

    float borderBeforeWidth() const;
    float borderAfterWidth() const;
    float borderStartWidth() const;
    float borderEndWidth() const;

    float outlineSize() const { return std::max<float>(0, outlineWidth() + outlineOffset()); }
    float outlineWidth() const;
    bool hasOutline() const { return outlineStyle() > BHIDDEN && outlineWidth() > 0; }
    EBorderStyle outlineStyle() const { return m_backgroundData->outline.style(); }
    OutlineIsAuto outlineStyleIsAuto() const { return static_cast<OutlineIsAuto>(m_backgroundData->outline.isAuto()); }
    bool hasOutlineInVisualOverflow() const { return hasOutline() && outlineSize() > 0; }
    
    EOverflow overflowX() const { return m_nonInheritedFlags.overflowX(); }
    EOverflow overflowY() const { return m_nonInheritedFlags.overflowY(); }

    EVisibility visibility() const { return static_cast<EVisibility>(m_inheritedFlags.visibility); }
    EVerticalAlign verticalAlign() const { return m_nonInheritedFlags.verticalAlign(); }
    const Length& verticalAlignLength() const { return m_boxData->verticalAlign(); }

    const Length& clipLeft() const { return m_visualData->clip.left(); }
    const Length& clipRight() const { return m_visualData->clip.right(); }
    const Length& clipTop() const { return m_visualData->clip.top(); }
    const Length& clipBottom() const { return m_visualData->clip.bottom(); }
    const LengthBox& clip() const { return m_visualData->clip; }
    bool hasClip() const { return m_visualData->hasClip; }

    EUnicodeBidi unicodeBidi() const { return m_nonInheritedFlags.unicodeBidi(); }

    EClear clear() const { return m_nonInheritedFlags.clear(); }
    ETableLayout tableLayout() const { return m_nonInheritedFlags.tableLayout(); }

    WEBCORE_EXPORT const FontCascade& fontCascade() const;
    WEBCORE_EXPORT const FontMetrics& fontMetrics() const;
    WEBCORE_EXPORT const FontCascadeDescription& fontDescription() const;
    float specifiedFontSize() const;
    float computedFontSize() const;
    int fontSize() const;
    std::pair<FontOrientation, NonCJKGlyphOrientation> fontAndGlyphOrientation();

#if ENABLE(VARIATION_FONTS)
    FontVariationSettings fontVariationSettings() const { return fontDescription().variationSettings(); }
#endif

    const Length& textIndent() const { return m_rareInheritedData->indent; }
    ETextAlign textAlign() const { return static_cast<ETextAlign>(m_inheritedFlags.textAlign); }
    ETextTransform textTransform() const { return static_cast<ETextTransform>(m_inheritedFlags.textTransform); }
    TextDecoration textDecorationsInEffect() const { return static_cast<TextDecoration>(m_inheritedFlags.textDecorations); }
    TextDecoration textDecoration() const { return static_cast<TextDecoration>(m_visualData->textDecoration); }
    TextDecorationStyle textDecorationStyle() const { return static_cast<TextDecorationStyle>(m_rareNonInheritedData->textDecorationStyle); }
    TextDecorationSkip textDecorationSkip() const { return static_cast<TextDecorationSkip>(m_rareInheritedData->textDecorationSkip); }
    TextUnderlinePosition textUnderlinePosition() const { return static_cast<TextUnderlinePosition>(m_rareInheritedData->textUnderlinePosition); }

#if ENABLE(CSS3_TEXT)
    TextIndentLine textIndentLine() const { return static_cast<TextIndentLine>(m_rareInheritedData->textIndentLine); }
    TextIndentType textIndentType() const { return static_cast<TextIndentType>(m_rareInheritedData->textIndentType); }
    TextAlignLast textAlignLast() const { return static_cast<TextAlignLast>(m_rareInheritedData->textAlignLast); }
    TextJustify textJustify() const { return static_cast<TextJustify>(m_rareInheritedData->textJustify); }
#endif

    const Length& wordSpacing() const;
    float letterSpacing() const;

    float zoom() const { return m_visualData->zoom; }
    float effectiveZoom() const { return m_rareInheritedData->effectiveZoom; }
    
    TextZoom textZoom() const { return static_cast<TextZoom>(m_rareInheritedData->textZoom); }

    TextDirection direction() const { return static_cast<TextDirection>(m_inheritedFlags.direction); }
    bool isLeftToRightDirection() const { return direction() == LTR; }
    bool hasExplicitlySetDirection() const { return m_nonInheritedFlags.hasExplicitlySetDirection(); }

    const Length& specifiedLineHeight() const;
    WEBCORE_EXPORT const Length& lineHeight() const;
    WEBCORE_EXPORT int computedLineHeight() const;

    EWhiteSpace whiteSpace() const { return static_cast<EWhiteSpace>(m_inheritedFlags.whiteSpace); }
    static bool autoWrap(EWhiteSpace);
    bool autoWrap() const { return autoWrap(whiteSpace()); }
    static bool preserveNewline(EWhiteSpace);
    bool preserveNewline() const { return preserveNewline(whiteSpace()); }
    static bool collapseWhiteSpace(EWhiteSpace);
    bool collapseWhiteSpace() const { return collapseWhiteSpace(whiteSpace()); }
    bool isCollapsibleWhiteSpace(UChar) const;
    bool breakOnlyAfterWhiteSpace() const;
    bool breakWords() const;

    EFillRepeat backgroundRepeatX() const { return static_cast<EFillRepeat>(m_backgroundData->background.repeatX()); }
    EFillRepeat backgroundRepeatY() const { return static_cast<EFillRepeat>(m_backgroundData->background.repeatY()); }
    CompositeOperator backgroundComposite() const { return static_cast<CompositeOperator>(m_backgroundData->background.composite()); }
    EFillAttachment backgroundAttachment() const { return static_cast<EFillAttachment>(m_backgroundData->background.attachment()); }
    EFillBox backgroundClip() const { return static_cast<EFillBox>(m_backgroundData->background.clip()); }
    EFillBox backgroundOrigin() const { return static_cast<EFillBox>(m_backgroundData->background.origin()); }
    const Length& backgroundXPosition() const { return m_backgroundData->background.xPosition(); }
    const Length& backgroundYPosition() const { return m_backgroundData->background.yPosition(); }
    EFillSizeType backgroundSizeType() const { return m_backgroundData->background.sizeType(); }
    const LengthSize& backgroundSizeLength() const { return m_backgroundData->background.sizeLength(); }
    FillLayer& ensureBackgroundLayers() { return m_backgroundData.access().background; }
    const FillLayer& backgroundLayers() const { return m_backgroundData->background; }

    StyleImage* maskImage() const { return m_rareNonInheritedData->mask.image(); }
    EFillRepeat maskRepeatX() const { return static_cast<EFillRepeat>(m_rareNonInheritedData->mask.repeatX()); }
    EFillRepeat maskRepeatY() const { return static_cast<EFillRepeat>(m_rareNonInheritedData->mask.repeatY()); }
    CompositeOperator maskComposite() const { return static_cast<CompositeOperator>(m_rareNonInheritedData->mask.composite()); }
    EFillBox maskClip() const { return static_cast<EFillBox>(m_rareNonInheritedData->mask.clip()); }
    EFillBox maskOrigin() const { return static_cast<EFillBox>(m_rareNonInheritedData->mask.origin()); }
    const Length& maskXPosition() const { return m_rareNonInheritedData->mask.xPosition(); }
    const Length& maskYPosition() const { return m_rareNonInheritedData->mask.yPosition(); }
    EFillSizeType maskSizeType() const { return m_rareNonInheritedData->mask.sizeType(); }
    const LengthSize& maskSizeLength() const { return m_rareNonInheritedData->mask.sizeLength(); }
    FillLayer& ensureMaskLayers() { return m_rareNonInheritedData.access().mask; }
    const FillLayer& maskLayers() const { return m_rareNonInheritedData->mask; }
    const NinePieceImage& maskBoxImage() const { return m_rareNonInheritedData->maskBoxImage; }
    StyleImage* maskBoxImageSource() const { return m_rareNonInheritedData->maskBoxImage.image(); }

    EBorderCollapse borderCollapse() const { return static_cast<EBorderCollapse>(m_inheritedFlags.borderCollapse); }
    float horizontalBorderSpacing() const;
    float verticalBorderSpacing() const;
    EEmptyCell emptyCells() const { return static_cast<EEmptyCell>(m_inheritedFlags.emptyCells); }
    ECaptionSide captionSide() const { return static_cast<ECaptionSide>(m_inheritedFlags.captionSide); }

    EListStyleType listStyleType() const { return static_cast<EListStyleType>(m_inheritedFlags.listStyleType); }
    StyleImage* listStyleImage() const;
    EListStylePosition listStylePosition() const { return static_cast<EListStylePosition>(m_inheritedFlags.listStylePosition); }

    const Length& marginTop() const { return m_surroundData->margin.top(); }
    const Length& marginBottom() const { return m_surroundData->margin.bottom(); }
    const Length& marginLeft() const { return m_surroundData->margin.left(); }
    const Length& marginRight() const { return m_surroundData->margin.right(); }
    const Length& marginBefore() const { return m_surroundData->margin.before(writingMode()); }
    const Length& marginAfter() const { return m_surroundData->margin.after(writingMode()); }
    const Length& marginStart() const { return m_surroundData->margin.start(writingMode(), direction()); }
    const Length& marginEnd() const { return m_surroundData->margin.end(writingMode(), direction()); }
    const Length& marginStartUsing(const RenderStyle* otherStyle) const { return m_surroundData->margin.start(otherStyle->writingMode(), otherStyle->direction()); }
    const Length& marginEndUsing(const RenderStyle* otherStyle) const { return m_surroundData->margin.end(otherStyle->writingMode(), otherStyle->direction()); }
    const Length& marginBeforeUsing(const RenderStyle* otherStyle) const { return m_surroundData->margin.before(otherStyle->writingMode()); }
    const Length& marginAfterUsing(const RenderStyle* otherStyle) const { return m_surroundData->margin.after(otherStyle->writingMode()); }

    const LengthBox& paddingBox() const { return m_surroundData->padding; }
    const Length& paddingTop() const { return m_surroundData->padding.top(); }
    const Length& paddingBottom() const { return m_surroundData->padding.bottom(); }
    const Length& paddingLeft() const { return m_surroundData->padding.left(); }
    const Length& paddingRight() const { return m_surroundData->padding.right(); }
    const Length& paddingBefore() const { return m_surroundData->padding.before(writingMode()); }
    const Length& paddingAfter() const { return m_surroundData->padding.after(writingMode()); }
    const Length& paddingStart() const { return m_surroundData->padding.start(writingMode(), direction()); }
    const Length& paddingEnd() const { return m_surroundData->padding.end(writingMode(), direction()); }

    ECursor cursor() const { return static_cast<ECursor>(m_inheritedFlags.cursor); }

#if ENABLE(CURSOR_VISIBILITY)
    CursorVisibility cursorVisibility() const { return static_cast<CursorVisibility>(m_inheritedFlags.cursorVisibility); }
#endif

    CursorList* cursors() const { return m_rareInheritedData->cursorData.get(); }

    EInsideLink insideLink() const { return static_cast<EInsideLink>(m_inheritedFlags.insideLink); }
    bool isLink() const { return m_nonInheritedFlags.isLink(); }

    bool insideDefaultButton() const { return m_inheritedFlags.insideDefaultButton; }

    short widows() const { return m_rareInheritedData->widows; }
    short orphans() const { return m_rareInheritedData->orphans; }
    bool hasAutoWidows() const { return m_rareInheritedData->hasAutoWidows; }
    bool hasAutoOrphans() const { return m_rareInheritedData->hasAutoOrphans; }

    BreakInside breakInside() const { return static_cast<BreakInside>(m_rareNonInheritedData->breakInside); }
    BreakBetween breakBefore() const { return static_cast<BreakBetween>(m_rareNonInheritedData->breakBefore); }
    BreakBetween breakAfter() const { return static_cast<BreakBetween>(m_rareNonInheritedData->breakAfter); }

    HangingPunctuation hangingPunctuation() const { return static_cast<HangingPunctuation>(m_rareInheritedData->hangingPunctuation); }

    float outlineOffset() const;
    const ShadowData* textShadow() const { return m_rareInheritedData->textShadow.get(); }
    void getTextShadowInlineDirectionExtent(LayoutUnit& logicalLeft, LayoutUnit& logicalRight) const { getShadowInlineDirectionExtent(textShadow(), logicalLeft, logicalRight); }
    void getTextShadowBlockDirectionExtent(LayoutUnit& logicalTop, LayoutUnit& logicalBottom) const { getShadowBlockDirectionExtent(textShadow(), logicalTop, logicalBottom); }

    float textStrokeWidth() const { return m_rareInheritedData->textStrokeWidth; }
    float opacity() const { return m_rareNonInheritedData->opacity; }
    ControlPart appearance() const { return static_cast<ControlPart>(m_rareNonInheritedData->appearance); }
    AspectRatioType aspectRatioType() const { return static_cast<AspectRatioType>(m_rareNonInheritedData->aspectRatioType); }
    float aspectRatioDenominator() const { return m_rareNonInheritedData->aspectRatioDenominator; }
    float aspectRatioNumerator() const { return m_rareNonInheritedData->aspectRatioNumerator; }
    EBoxAlignment boxAlign() const { return static_cast<EBoxAlignment>(m_rareNonInheritedData->deprecatedFlexibleBox->align); }
    EBoxDirection boxDirection() const { return static_cast<EBoxDirection>(m_inheritedFlags.boxDirection); }
    float boxFlex() const { return m_rareNonInheritedData->deprecatedFlexibleBox->flex; }
    unsigned boxFlexGroup() const { return m_rareNonInheritedData->deprecatedFlexibleBox->flexGroup; }
    EBoxLines boxLines() const { return static_cast<EBoxLines>(m_rareNonInheritedData->deprecatedFlexibleBox->lines); }
    unsigned boxOrdinalGroup() const { return m_rareNonInheritedData->deprecatedFlexibleBox->ordinalGroup; }
    EBoxOrient boxOrient() const { return static_cast<EBoxOrient>(m_rareNonInheritedData->deprecatedFlexibleBox->orient); }
    EBoxPack boxPack() const { return static_cast<EBoxPack>(m_rareNonInheritedData->deprecatedFlexibleBox->pack); }

    int order() const { return m_rareNonInheritedData->order; }
    float flexGrow() const { return m_rareNonInheritedData->flexibleBox->flexGrow; }
    float flexShrink() const { return m_rareNonInheritedData->flexibleBox->flexShrink; }
    const Length& flexBasis() const { return m_rareNonInheritedData->flexibleBox->flexBasis; }
    const StyleContentAlignmentData& alignContent() const { return m_rareNonInheritedData->alignContent; }
    const StyleSelfAlignmentData& alignItems() const { return m_rareNonInheritedData->alignItems; }
    const StyleSelfAlignmentData& alignSelf() const { return m_rareNonInheritedData->alignSelf; }
    EFlexDirection flexDirection() const { return static_cast<EFlexDirection>(m_rareNonInheritedData->flexibleBox->flexDirection); }
    bool isColumnFlexDirection() const { return flexDirection() == FlowColumn || flexDirection() == FlowColumnReverse; }
    bool isReverseFlexDirection() const { return flexDirection() == FlowRowReverse || flexDirection() == FlowColumnReverse; }
    EFlexWrap flexWrap() const { return static_cast<EFlexWrap>(m_rareNonInheritedData->flexibleBox->flexWrap); }
    const StyleContentAlignmentData& justifyContent() const { return m_rareNonInheritedData->justifyContent; }
    const StyleSelfAlignmentData& justifyItems() const { return m_rareNonInheritedData->justifyItems; }
    const StyleSelfAlignmentData& justifySelf() const { return m_rareNonInheritedData->justifySelf; }

#if ENABLE(CSS_GRID_LAYOUT)
    const Vector<GridTrackSize>& gridColumns() const { return m_rareNonInheritedData->grid->gridColumns; }
    const Vector<GridTrackSize>& gridRows() const { return m_rareNonInheritedData->grid->gridRows; }
    const Vector<GridTrackSize>& gridAutoRepeatColumns() const { return m_rareNonInheritedData->grid->gridAutoRepeatColumns; }
    const Vector<GridTrackSize>& gridAutoRepeatRows() const { return m_rareNonInheritedData->grid->gridAutoRepeatRows; }
    unsigned gridAutoRepeatColumnsInsertionPoint() const { return m_rareNonInheritedData->grid->autoRepeatColumnsInsertionPoint; }
    unsigned gridAutoRepeatRowsInsertionPoint() const { return m_rareNonInheritedData->grid->autoRepeatRowsInsertionPoint; }
    AutoRepeatType gridAutoRepeatColumnsType() const  { return m_rareNonInheritedData->grid->autoRepeatColumnsType; }
    AutoRepeatType gridAutoRepeatRowsType() const  { return m_rareNonInheritedData->grid->autoRepeatRowsType; }
    const NamedGridLinesMap& namedGridColumnLines() const { return m_rareNonInheritedData->grid->namedGridColumnLines; }
    const NamedGridLinesMap& namedGridRowLines() const { return m_rareNonInheritedData->grid->namedGridRowLines; }
    const OrderedNamedGridLinesMap& orderedNamedGridColumnLines() const { return m_rareNonInheritedData->grid->orderedNamedGridColumnLines; }
    const OrderedNamedGridLinesMap& orderedNamedGridRowLines() const { return m_rareNonInheritedData->grid->orderedNamedGridRowLines; }
    const NamedGridLinesMap& autoRepeatNamedGridColumnLines() const { return m_rareNonInheritedData->grid->autoRepeatNamedGridColumnLines; }
    const NamedGridLinesMap& autoRepeatNamedGridRowLines() const { return m_rareNonInheritedData->grid->autoRepeatNamedGridRowLines; }
    const OrderedNamedGridLinesMap& autoRepeatOrderedNamedGridColumnLines() const { return m_rareNonInheritedData->grid->autoRepeatOrderedNamedGridColumnLines; }
    const OrderedNamedGridLinesMap& autoRepeatOrderedNamedGridRowLines() const { return m_rareNonInheritedData->grid->autoRepeatOrderedNamedGridRowLines; }
    const NamedGridAreaMap& namedGridArea() const { return m_rareNonInheritedData->grid->namedGridArea; }
    size_t namedGridAreaRowCount() const { return m_rareNonInheritedData->grid->namedGridAreaRowCount; }
    size_t namedGridAreaColumnCount() const { return m_rareNonInheritedData->grid->namedGridAreaColumnCount; }
    GridAutoFlow gridAutoFlow() const { return static_cast<GridAutoFlow>(m_rareNonInheritedData->grid->gridAutoFlow); }
    bool isGridAutoFlowDirectionRow() const { return (m_rareNonInheritedData->grid->gridAutoFlow & InternalAutoFlowDirectionRow); }
    bool isGridAutoFlowDirectionColumn() const { return (m_rareNonInheritedData->grid->gridAutoFlow & InternalAutoFlowDirectionColumn); }
    bool isGridAutoFlowAlgorithmSparse() const { return (m_rareNonInheritedData->grid->gridAutoFlow & InternalAutoFlowAlgorithmSparse); }
    bool isGridAutoFlowAlgorithmDense() const { return (m_rareNonInheritedData->grid->gridAutoFlow & InternalAutoFlowAlgorithmDense); }
    const Vector<GridTrackSize>& gridAutoColumns() const { return m_rareNonInheritedData->grid->gridAutoColumns; }
    const Vector<GridTrackSize>& gridAutoRows() const { return m_rareNonInheritedData->grid->gridAutoRows; }
    const Length& gridColumnGap() const { return m_rareNonInheritedData->grid->gridColumnGap; }
    const Length& gridRowGap() const { return m_rareNonInheritedData->grid->gridRowGap; }

    const GridPosition& gridItemColumnStart() const { return m_rareNonInheritedData->gridItem->gridColumnStart; }
    const GridPosition& gridItemColumnEnd() const { return m_rareNonInheritedData->gridItem->gridColumnEnd; }
    const GridPosition& gridItemRowStart() const { return m_rareNonInheritedData->gridItem->gridRowStart; }
    const GridPosition& gridItemRowEnd() const { return m_rareNonInheritedData->gridItem->gridRowEnd; }
#endif // ENABLE(CSS_GRID_LAYOUT)

    const ShadowData* boxShadow() const { return m_rareNonInheritedData->boxShadow.get(); }
    void getBoxShadowExtent(LayoutUnit& top, LayoutUnit& right, LayoutUnit& bottom, LayoutUnit& left) const { getShadowExtent(boxShadow(), top, right, bottom, left); }
    LayoutBoxExtent getBoxShadowInsetExtent() const { return getShadowInsetExtent(boxShadow()); }
    void getBoxShadowHorizontalExtent(LayoutUnit& left, LayoutUnit& right) const { getShadowHorizontalExtent(boxShadow(), left, right); }
    void getBoxShadowVerticalExtent(LayoutUnit& top, LayoutUnit& bottom) const { getShadowVerticalExtent(boxShadow(), top, bottom); }
    void getBoxShadowInlineDirectionExtent(LayoutUnit& logicalLeft, LayoutUnit& logicalRight) const { getShadowInlineDirectionExtent(boxShadow(), logicalLeft, logicalRight); }
    void getBoxShadowBlockDirectionExtent(LayoutUnit& logicalTop, LayoutUnit& logicalBottom) const { getShadowBlockDirectionExtent(boxShadow(), logicalTop, logicalBottom); }

#if ENABLE(CSS_BOX_DECORATION_BREAK)
    EBoxDecorationBreak boxDecorationBreak() const { return m_boxData->boxDecorationBreak(); }
#endif

    StyleReflection* boxReflect() const { return m_rareNonInheritedData->boxReflect.get(); }
    EBoxSizing boxSizing() const { return m_boxData->boxSizing(); }
    const Length& marqueeIncrement() const { return m_rareNonInheritedData->marquee->increment; }
    int marqueeSpeed() const { return m_rareNonInheritedData->marquee->speed; }
    int marqueeLoopCount() const { return m_rareNonInheritedData->marquee->loops; }
    EMarqueeBehavior marqueeBehavior() const { return static_cast<EMarqueeBehavior>(m_rareNonInheritedData->marquee->behavior); }
    EMarqueeDirection marqueeDirection() const { return static_cast<EMarqueeDirection>(m_rareNonInheritedData->marquee->direction); }
    EUserModify userModify() const { return static_cast<EUserModify>(m_rareInheritedData->userModify); }
    EUserDrag userDrag() const { return static_cast<EUserDrag>(m_rareNonInheritedData->userDrag); }
    EUserSelect userSelect() const { return static_cast<EUserSelect>(m_rareInheritedData->userSelect); }
    TextOverflow textOverflow() const { return static_cast<TextOverflow>(m_rareNonInheritedData->textOverflow); }
    EMarginCollapse marginBeforeCollapse() const { return static_cast<EMarginCollapse>(m_rareNonInheritedData->marginBeforeCollapse); }
    EMarginCollapse marginAfterCollapse() const { return static_cast<EMarginCollapse>(m_rareNonInheritedData->marginAfterCollapse); }
    EWordBreak wordBreak() const { return static_cast<EWordBreak>(m_rareInheritedData->wordBreak); }
    EOverflowWrap overflowWrap() const { return static_cast<EOverflowWrap>(m_rareInheritedData->overflowWrap); }
    ENBSPMode nbspMode() const { return static_cast<ENBSPMode>(m_rareInheritedData->nbspMode); }
    LineBreak lineBreak() const { return static_cast<LineBreak>(m_rareInheritedData->lineBreak); }
    Hyphens hyphens() const { return static_cast<Hyphens>(m_rareInheritedData->hyphens); }
    short hyphenationLimitBefore() const { return m_rareInheritedData->hyphenationLimitBefore; }
    short hyphenationLimitAfter() const { return m_rareInheritedData->hyphenationLimitAfter; }
    short hyphenationLimitLines() const { return m_rareInheritedData->hyphenationLimitLines; }
    const AtomicString& hyphenationString() const { return m_rareInheritedData->hyphenationString; }
    const AtomicString& locale() const { return fontDescription().locale(); }
    EBorderFit borderFit() const { return static_cast<EBorderFit>(m_rareNonInheritedData->borderFit); }
    EResize resize() const { return static_cast<EResize>(m_rareNonInheritedData->resize); }
    ColumnAxis columnAxis() const { return static_cast<ColumnAxis>(m_rareNonInheritedData->multiCol->axis); }
    bool hasInlineColumnAxis() const;
    ColumnProgression columnProgression() const { return static_cast<ColumnProgression>(m_rareNonInheritedData->multiCol->progression); }
    float columnWidth() const { return m_rareNonInheritedData->multiCol->width; }
    bool hasAutoColumnWidth() const { return m_rareNonInheritedData->multiCol->autoWidth; }
    unsigned short columnCount() const { return m_rareNonInheritedData->multiCol->count; }
    bool hasAutoColumnCount() const { return m_rareNonInheritedData->multiCol->autoCount; }
    bool specifiesColumns() const { return !hasAutoColumnCount() || !hasAutoColumnWidth() || !hasInlineColumnAxis(); }
    ColumnFill columnFill() const { return static_cast<ColumnFill>(m_rareNonInheritedData->multiCol->fill); }
    float columnGap() const { return m_rareNonInheritedData->multiCol->gap; }
    bool hasNormalColumnGap() const { return m_rareNonInheritedData->multiCol->normalGap; }
    EBorderStyle columnRuleStyle() const { return m_rareNonInheritedData->multiCol->rule.style(); }
    unsigned short columnRuleWidth() const { return m_rareNonInheritedData->multiCol->ruleWidth(); }
    bool columnRuleIsTransparent() const { return m_rareNonInheritedData->multiCol->rule.isTransparent(); }
    ColumnSpan columnSpan() const { return static_cast<ColumnSpan>(m_rareNonInheritedData->multiCol->columnSpan); }

    const TransformOperations& transform() const { return m_rareNonInheritedData->transform->operations; }
    const Length& transformOriginX() const { return m_rareNonInheritedData->transform->x; }
    const Length& transformOriginY() const { return m_rareNonInheritedData->transform->y; }
    float transformOriginZ() const { return m_rareNonInheritedData->transform->z; }
    bool hasTransform() const { return !m_rareNonInheritedData->transform->operations.operations().isEmpty(); }

    TextEmphasisFill textEmphasisFill() const { return static_cast<TextEmphasisFill>(m_rareInheritedData->textEmphasisFill); }
    TextEmphasisMark textEmphasisMark() const;
    const AtomicString& textEmphasisCustomMark() const { return m_rareInheritedData->textEmphasisCustomMark; }
    TextEmphasisPosition textEmphasisPosition() const { return static_cast<TextEmphasisPosition>(m_rareInheritedData->textEmphasisPosition); }
    const AtomicString& textEmphasisMarkString() const;

    RubyPosition rubyPosition() const { return static_cast<RubyPosition>(m_rareInheritedData->rubyPosition); }

    TextOrientation textOrientation() const { return static_cast<TextOrientation>(m_rareInheritedData->textOrientation); }

    ObjectFit objectFit() const { return static_cast<ObjectFit>(m_rareNonInheritedData->objectFit); }
    LengthPoint objectPosition() const { return m_rareNonInheritedData->objectPosition; }

    // Return true if any transform related property (currently transform, transformStyle3D or perspective)
    // indicates that we are transforming.
    bool hasTransformRelatedProperty() const { return hasTransform() || preserves3D() || hasPerspective(); }

    enum ApplyTransformOrigin { IncludeTransformOrigin, ExcludeTransformOrigin };
    void applyTransform(TransformationMatrix&, const FloatRect& boundingBox, ApplyTransformOrigin = IncludeTransformOrigin) const;
    void setPageScaleTransform(float);

    bool hasMask() const { return m_rareNonInheritedData->mask.hasImage() || m_rareNonInheritedData->maskBoxImage.hasImage(); }

    TextCombine textCombine() const { return static_cast<TextCombine>(m_rareNonInheritedData->textCombine); }
    bool hasTextCombine() const { return textCombine() != TextCombineNone; }

    unsigned tabSize() const { return m_rareInheritedData->tabSize; }

    // End CSS3 Getters

    bool hasFlowInto() const { return !m_rareNonInheritedData->flowThread.isNull(); }
    const AtomicString& flowThread() const { return m_rareNonInheritedData->flowThread; }
    bool hasFlowFrom() const { return !m_rareNonInheritedData->regionThread.isNull(); }
    const AtomicString& regionThread() const { return m_rareNonInheritedData->regionThread; }
    RegionFragment regionFragment() const { return static_cast<RegionFragment>(m_rareNonInheritedData->regionFragment); }

    const AtomicString& lineGrid() const { return m_rareInheritedData->lineGrid; }
    LineSnap lineSnap() const { return static_cast<LineSnap>(m_rareInheritedData->lineSnap); }
    LineAlign lineAlign() const { return static_cast<LineAlign>(m_rareInheritedData->lineAlign); }

    EPointerEvents pointerEvents() const { return static_cast<EPointerEvents>(m_inheritedFlags.pointerEvents); }
    const AnimationList* animations() const { return m_rareNonInheritedData->animations.get(); }
    const AnimationList* transitions() const { return m_rareNonInheritedData->transitions.get(); }

    AnimationList* animations() { return m_rareNonInheritedData->animations.get(); }
    AnimationList* transitions() { return m_rareNonInheritedData->transitions.get(); }
    
    bool hasAnimationsOrTransitions() const { return m_rareNonInheritedData->hasAnimationsOrTransitions(); }

    AnimationList& ensureAnimations();
    AnimationList& ensureTransitions();

    bool hasAnimations() const { return m_rareNonInheritedData->animations && m_rareNonInheritedData->animations->size() > 0; }
    bool hasTransitions() const { return m_rareNonInheritedData->transitions && m_rareNonInheritedData->transitions->size() > 0; }

    // Return the first found Animation (including 'all' transitions).
    const Animation* transitionForProperty(CSSPropertyID) const;

    ETransformStyle3D transformStyle3D() const { return static_cast<ETransformStyle3D>(m_rareNonInheritedData->transformStyle3D); }
    bool preserves3D() const { return m_rareNonInheritedData->transformStyle3D == TransformStyle3DPreserve3D; }

    EBackfaceVisibility backfaceVisibility() const { return static_cast<EBackfaceVisibility>(m_rareNonInheritedData->backfaceVisibility); }
    float perspective() const { return m_rareNonInheritedData->perspective; }
    bool hasPerspective() const { return m_rareNonInheritedData->perspective > 0; }
    const Length& perspectiveOriginX() const { return m_rareNonInheritedData->perspectiveOriginX; }
    const Length& perspectiveOriginY() const { return m_rareNonInheritedData->perspectiveOriginY; }
    const LengthSize& pageSize() const { return m_rareNonInheritedData->pageSize; }
    PageSizeType pageSizeType() const { return static_cast<PageSizeType>(m_rareNonInheritedData->pageSizeType); }
    
    LineBoxContain lineBoxContain() const { return m_rareInheritedData->lineBoxContain; }
    const LineClampValue& lineClamp() const { return m_rareNonInheritedData->lineClamp; }
    const IntSize& initialLetter() const { return m_rareNonInheritedData->initialLetter; }
    int initialLetterDrop() const { return initialLetter().width(); }
    int initialLetterHeight() const { return initialLetter().height(); }

#if ENABLE(TOUCH_EVENTS)
    TouchAction touchAction() const { return static_cast<TouchAction>(m_rareNonInheritedData->touchAction); }
#endif

#if ENABLE(CSS_SCROLL_SNAP)
    // Scroll snap port style.
    const StyleScrollSnapPort& scrollSnapPort() const;
    const ScrollSnapType& scrollSnapType() const;
    const LengthBox& scrollPadding() const;
    const Length& scrollPaddingTop() const;
    const Length& scrollPaddingBottom() const;
    const Length& scrollPaddingLeft() const;
    const Length& scrollPaddingRight() const;

    // Scroll snap area style.
    const StyleScrollSnapArea& scrollSnapArea() const;
    const ScrollSnapAlign& scrollSnapAlign() const;
    const LengthBox& scrollSnapMargin() const;
    const Length& scrollSnapMarginTop() const;
    const Length& scrollSnapMarginBottom() const;
    const Length& scrollSnapMarginLeft() const;
    const Length& scrollSnapMarginRight() const;
#endif

#if ENABLE(TOUCH_EVENTS)
    Color tapHighlightColor() const { return m_rareInheritedData->tapHighlightColor; }
#endif

#if PLATFORM(IOS)
    bool touchCalloutEnabled() const { return m_rareInheritedData->touchCalloutEnabled; }
#endif

#if ENABLE(ACCELERATED_OVERFLOW_SCROLLING)
    bool useTouchOverflowScrolling() const { return m_rareInheritedData->useTouchOverflowScrolling; }
#endif

#if ENABLE(TEXT_AUTOSIZING)
    TextSizeAdjustment textSizeAdjust() const { return m_rareInheritedData->textSizeAdjust; }
#endif

    ETextSecurity textSecurity() const { return static_cast<ETextSecurity>(m_rareInheritedData->textSecurity); }

    WritingMode writingMode() const { return static_cast<WritingMode>(m_inheritedFlags.writingMode); }
    bool isHorizontalWritingMode() const { return WebCore::isHorizontalWritingMode(writingMode()); }
    bool isVerticalWritingMode() const { return WebCore::isVerticalWritingMode(writingMode()); }
    bool isFlippedLinesWritingMode() const { return WebCore::isFlippedLinesWritingMode(writingMode()); }
    bool isFlippedBlocksWritingMode() const { return WebCore::isFlippedWritingMode(writingMode()); }

    ImageOrientationEnum imageOrientation() const;

    EImageRendering imageRendering() const { return static_cast<EImageRendering>(m_rareInheritedData->imageRendering); }

#if ENABLE(CSS_IMAGE_RESOLUTION)
    ImageResolutionSource imageResolutionSource() const { return static_cast<ImageResolutionSource>(m_rareInheritedData->imageResolutionSource); }
    ImageResolutionSnap imageResolutionSnap() const { return static_cast<ImageResolutionSnap>(m_rareInheritedData->imageResolutionSnap); }
    float imageResolution() const { return m_rareInheritedData->imageResolution; }
#endif
    
    ESpeak speak() const { return static_cast<ESpeak>(m_rareInheritedData->speak); }

    FilterOperations& mutableFilter() { return m_rareNonInheritedData.access().filter.access().operations; }
    const FilterOperations& filter() const { return m_rareNonInheritedData->filter->operations; }
    bool hasFilter() const { return !m_rareNonInheritedData->filter->operations.operations().isEmpty(); }
    bool hasReferenceFilterOnly() const;

#if ENABLE(FILTERS_LEVEL_2)
    FilterOperations& mutableBackdropFilter() { return m_rareNonInheritedData.access().backdropFilter.access().operations; }
    const FilterOperations& backdropFilter() const { return m_rareNonInheritedData->backdropFilter->operations; }
    bool hasBackdropFilter() const { return !m_rareNonInheritedData->backdropFilter->operations.operations().isEmpty(); }
#else
    bool hasBackdropFilter() const { return false; }
#endif

#if ENABLE(CSS_COMPOSITING)
    BlendMode blendMode() const { return static_cast<BlendMode>(m_rareNonInheritedData->effectiveBlendMode); }
    void setBlendMode(BlendMode mode) { SET_VAR(m_rareNonInheritedData, effectiveBlendMode, mode); }
    bool hasBlendMode() const { return static_cast<BlendMode>(m_rareNonInheritedData->effectiveBlendMode) != BlendModeNormal; }

    Isolation isolation() const { return static_cast<Isolation>(m_rareNonInheritedData->isolation); }
    void setIsolation(Isolation isolation) { SET_VAR(m_rareNonInheritedData, isolation, isolation); }
    bool hasIsolation() const { return m_rareNonInheritedData->isolation != IsolationAuto; }
#else
    BlendMode blendMode() const { return BlendModeNormal; }
    bool hasBlendMode() const { return false; }

    Isolation isolation() const { return IsolationAuto; }
    bool hasIsolation() const { return false; }
#endif

    bool shouldPlaceBlockDirectionScrollbarOnLeft() const;

#if ENABLE(CSS_TRAILING_WORD)
    TrailingWord trailingWord() const { return static_cast<TrailingWord>(m_rareInheritedData->trailingWord); }
#endif

#if ENABLE(APPLE_PAY)
    ApplePayButtonStyle applePayButtonStyle() const { return static_cast<ApplePayButtonStyle>(m_rareNonInheritedData->applePayButtonStyle); }
    ApplePayButtonType applePayButtonType() const { return static_cast<ApplePayButtonType>(m_rareNonInheritedData->applePayButtonType); }
#endif

    void checkVariablesInCustomProperties();

// attribute setter methods

    void setDisplay(EDisplay v) { m_nonInheritedFlags.setEffectiveDisplay(v); }
    void setOriginalDisplay(EDisplay v) { m_nonInheritedFlags.setOriginalDisplay(v); }
    void setPosition(EPosition v) { m_nonInheritedFlags.setPosition(v); }
    void setFloating(EFloat v) { m_nonInheritedFlags.setFloating(v); }

    void setLeft(Length&& length) { SET_VAR(m_surroundData, offset.left(), WTFMove(length)); }
    void setRight(Length&& length) { SET_VAR(m_surroundData, offset.right(), WTFMove(length)); }
    void setTop(Length&& length) { SET_VAR(m_surroundData, offset.top(), WTFMove(length)); }
    void setBottom(Length&& length) { SET_VAR(m_surroundData, offset.bottom(), WTFMove(length)); }

    void setWidth(Length&& length) { SET_VAR(m_boxData, m_width, WTFMove(length)); }
    void setHeight(Length&& length) { SET_VAR(m_boxData, m_height, WTFMove(length)); }

    void setLogicalWidth(Length&&);
    void setLogicalHeight(Length&&);

    void setMinWidth(Length&& length) { SET_VAR(m_boxData, m_minWidth, WTFMove(length)); }
    void setMaxWidth(Length&& length) { SET_VAR(m_boxData, m_maxWidth, WTFMove(length)); }
    void setMinHeight(Length&& length) { SET_VAR(m_boxData, m_minHeight, WTFMove(length)); }
    void setMaxHeight(Length&& length) { SET_VAR(m_boxData, m_maxHeight, WTFMove(length)); }

#if ENABLE(DASHBOARD_SUPPORT)
    const Vector<StyleDashboardRegion>& dashboardRegions() const { return m_rareNonInheritedData->dashboardRegions; }
    void setDashboardRegions(const Vector<StyleDashboardRegion>& regions) { SET_VAR(m_rareNonInheritedData, dashboardRegions, regions); }
    void setDashboardRegion(int type, const String& label, Length&& top, Length&& right, Length&& bottom, Length&& left, bool append);
#endif

    void resetBorder() { resetBorderImage(); resetBorderTop(); resetBorderRight(); resetBorderBottom(); resetBorderLeft(); resetBorderRadius(); }
    void resetBorderTop() { SET_VAR(m_surroundData, border.m_top, BorderValue()); }
    void resetBorderRight() { SET_VAR(m_surroundData, border.m_right, BorderValue()); }
    void resetBorderBottom() { SET_VAR(m_surroundData, border.m_bottom, BorderValue()); }
    void resetBorderLeft() { SET_VAR(m_surroundData, border.m_left, BorderValue()); }
    void resetBorderImage() { SET_VAR(m_surroundData, border.m_image, NinePieceImage()); }
    void resetBorderRadius() { resetBorderTopLeftRadius(); resetBorderTopRightRadius(); resetBorderBottomLeftRadius(); resetBorderBottomRightRadius(); }
    void resetBorderTopLeftRadius() { SET_VAR(m_surroundData, border.m_topLeft, initialBorderRadius()); }
    void resetBorderTopRightRadius() { SET_VAR(m_surroundData, border.m_topRight, initialBorderRadius()); }
    void resetBorderBottomLeftRadius() { SET_VAR(m_surroundData, border.m_bottomLeft, initialBorderRadius()); }
    void resetBorderBottomRightRadius() { SET_VAR(m_surroundData, border.m_bottomRight, initialBorderRadius()); }

    void setBackgroundColor(const Color& v) { SET_VAR(m_backgroundData, color, v); }

    void setBackgroundXPosition(Length&& length) { SET_VAR(m_backgroundData, background.m_xPosition, WTFMove(length)); }
    void setBackgroundYPosition(Length&& length) { SET_VAR(m_backgroundData, background.m_yPosition, WTFMove(length)); }
    void setBackgroundSize(EFillSizeType b) { SET_VAR(m_backgroundData, background.m_sizeType, b); }
    void setBackgroundSizeLength(LengthSize&& size) { SET_VAR(m_backgroundData, background.m_sizeLength, WTFMove(size)); }
    
    void setBorderImage(const NinePieceImage& b) { SET_VAR(m_surroundData, border.m_image, b); }
    void setBorderImageSource(RefPtr<StyleImage>&&);
    void setBorderImageSlices(LengthBox&&);
    void setBorderImageWidth(LengthBox&&);
    void setBorderImageOutset(LengthBox&&);

    void setBorderTopLeftRadius(LengthSize&& size) { SET_VAR(m_surroundData, border.m_topLeft, WTFMove(size)); }
    void setBorderTopRightRadius(LengthSize&& size) { SET_VAR(m_surroundData, border.m_topRight, WTFMove(size)); }
    void setBorderBottomLeftRadius(LengthSize&& size) { SET_VAR(m_surroundData, border.m_bottomLeft, WTFMove(size)); }
    void setBorderBottomRightRadius(LengthSize&& size) { SET_VAR(m_surroundData, border.m_bottomRight, WTFMove(size)); }

    void setBorderRadius(LengthSize&&);
    void setBorderRadius(const IntSize&);

    RoundedRect getRoundedBorderFor(const LayoutRect& borderRect, bool includeLogicalLeftEdge = true, bool includeLogicalRightEdge = true) const;
    RoundedRect getRoundedInnerBorderFor(const LayoutRect& borderRect, bool includeLogicalLeftEdge = true, bool includeLogicalRightEdge = true) const;

    RoundedRect getRoundedInnerBorderFor(const LayoutRect& borderRect, LayoutUnit topWidth, LayoutUnit bottomWidth,
        LayoutUnit leftWidth, LayoutUnit rightWidth, bool includeLogicalLeftEdge = true, bool includeLogicalRightEdge = true) const;

    void setBorderLeftWidth(float v) { SET_VAR(m_surroundData, border.m_left.m_width, v); }
    void setBorderLeftStyle(EBorderStyle v) { SET_VAR(m_surroundData, border.m_left.m_style, v); }
    void setBorderLeftColor(const Color& v) { SET_BORDERVALUE_COLOR(m_surroundData, border.m_left, v); }
    void setBorderRightWidth(float v) { SET_VAR(m_surroundData, border.m_right.m_width, v); }
    void setBorderRightStyle(EBorderStyle v) { SET_VAR(m_surroundData, border.m_right.m_style, v); }
    void setBorderRightColor(const Color& v) { SET_BORDERVALUE_COLOR(m_surroundData, border.m_right, v); }
    void setBorderTopWidth(float v) { SET_VAR(m_surroundData, border.m_top.m_width, v); }
    void setBorderTopStyle(EBorderStyle v) { SET_VAR(m_surroundData, border.m_top.m_style, v); }
    void setBorderTopColor(const Color& v) { SET_BORDERVALUE_COLOR(m_surroundData, border.m_top, v); }
    void setBorderBottomWidth(float v) { SET_VAR(m_surroundData, border.m_bottom.m_width, v); }
    void setBorderBottomStyle(EBorderStyle v) { SET_VAR(m_surroundData, border.m_bottom.m_style, v); }
    void setBorderBottomColor(const Color& v) { SET_BORDERVALUE_COLOR(m_surroundData, border.m_bottom, v); }

    void setOutlineWidth(float v) { SET_VAR(m_backgroundData, outline.m_width, v); }
    void setOutlineStyleIsAuto(OutlineIsAuto isAuto) { SET_VAR(m_backgroundData, outline.m_isAuto, isAuto); }
    void setOutlineStyle(EBorderStyle v) { SET_VAR(m_backgroundData, outline.m_style, v); }
    void setOutlineColor(const Color& v) { SET_BORDERVALUE_COLOR(m_backgroundData, outline, v); }

    void setOverflowX(EOverflow v) { m_nonInheritedFlags.setOverflowX(v); }
    void setOverflowY(EOverflow v) { m_nonInheritedFlags.setOverflowY(v); }
    void setVisibility(EVisibility v) { m_inheritedFlags.visibility = v; }
    void setVerticalAlign(EVerticalAlign v) { m_nonInheritedFlags.setVerticalAlign(v); }
    void setVerticalAlignLength(Length&& length) { setVerticalAlign(LENGTH); SET_VAR(m_boxData, m_verticalAlign, WTFMove(length)); }

    void setHasClip(bool b = true) { SET_VAR(m_visualData, hasClip, b); }
    void setClipLeft(Length&& length) { SET_VAR(m_visualData, clip.left(), WTFMove(length)); }
    void setClipRight(Length&& length) { SET_VAR(m_visualData, clip.right(), WTFMove(length)); }
    void setClipTop(Length&& length) { SET_VAR(m_visualData, clip.top(), WTFMove(length)); }
    void setClipBottom(Length&& length) { SET_VAR(m_visualData, clip.bottom(), WTFMove(length)); }
    void setClip(Length&& top, Length&& right, Length&& bottom, Length&& left);
    void setClip(LengthBox&& box) { SET_VAR(m_visualData, clip, WTFMove(box)); }

    void setUnicodeBidi(EUnicodeBidi b) { m_nonInheritedFlags.setUnicodeBidi(b); }

    void setClear(EClear v) { m_nonInheritedFlags.setClear(v); }
    void setTableLayout(ETableLayout v) { m_nonInheritedFlags.setTableLayout(v); }

    bool setFontDescription(const FontCascadeDescription&);

    // Only used for blending font sizes when animating, for MathML anonymous blocks, and for text autosizing.
    void setFontSize(float);

#if ENABLE(VARIATION_FONTS)
    void setFontVariationSettings(FontVariationSettings);
#endif

    void setColor(const Color&);
    void setTextIndent(Length&& length) { SET_VAR(m_rareInheritedData, indent, WTFMove(length)); }
    void setTextAlign(ETextAlign v) { m_inheritedFlags.textAlign = v; }
    void setTextTransform(ETextTransform v) { m_inheritedFlags.textTransform = v; }
    void addToTextDecorationsInEffect(TextDecoration v) { m_inheritedFlags.textDecorations |= v; }
    void setTextDecorationsInEffect(TextDecoration v) { m_inheritedFlags.textDecorations = v; }
    void setTextDecoration(TextDecoration v) { SET_VAR(m_visualData, textDecoration, v); }
    void setTextDecorationStyle(TextDecorationStyle v) { SET_VAR(m_rareNonInheritedData, textDecorationStyle, v); }
    void setTextDecorationSkip(TextDecorationSkip skip) { SET_VAR(m_rareInheritedData, textDecorationSkip, skip); }
    void setTextUnderlinePosition(TextUnderlinePosition v) { SET_VAR(m_rareInheritedData, textUnderlinePosition, v); }
    void setDirection(TextDirection v) { m_inheritedFlags.direction = v; }
    void setHasExplicitlySetDirection(bool v) { m_nonInheritedFlags.setHasExplicitlySetDirection(v); }
    void setLineHeight(Length&&);
    bool setZoom(float);
    void setZoomWithoutReturnValue(float f) { setZoom(f); }
    bool setEffectiveZoom(float);
    void setTextZoom(TextZoom v) { SET_VAR(m_rareInheritedData, textZoom, v); }

#if ENABLE(CSS3_TEXT)
    void setTextIndentLine(TextIndentLine v) { SET_VAR(m_rareInheritedData, textIndentLine, v); }
    void setTextIndentType(TextIndentType v) { SET_VAR(m_rareInheritedData, textIndentType, v); }
    void setTextAlignLast(TextAlignLast v) { SET_VAR(m_rareInheritedData, textAlignLast, v); }
    void setTextJustify(TextJustify v) { SET_VAR(m_rareInheritedData, textJustify, v); }
#endif

#if ENABLE(TEXT_AUTOSIZING)
    void setSpecifiedLineHeight(Length&&);
#endif

#if ENABLE(CSS_IMAGE_ORIENTATION)
    void setImageOrientation(ImageOrientationEnum v) { SET_VAR(m_rareInheritedData, imageOrientation, static_cast<int>(v)); }
#endif

    void setImageRendering(EImageRendering v) { SET_VAR(m_rareInheritedData, imageRendering, v); }

#if ENABLE(CSS_IMAGE_RESOLUTION)
    void setImageResolutionSource(ImageResolutionSource v) { SET_VAR(m_rareInheritedData, imageResolutionSource, v); }
    void setImageResolutionSnap(ImageResolutionSnap v) { SET_VAR(m_rareInheritedData, imageResolutionSnap, v); }
    void setImageResolution(float f) { SET_VAR(m_rareInheritedData, imageResolution, f); }
#endif

    void setWhiteSpace(EWhiteSpace v) { m_inheritedFlags.whiteSpace = v; }

    void setWordSpacing(Length&&);
    void setLetterSpacing(float);

    void clearBackgroundLayers() { m_backgroundData.access().background = FillLayer(BackgroundFillLayer); }
    void inheritBackgroundLayers(const FillLayer& parent) { m_backgroundData.access().background = parent; }

    void adjustBackgroundLayers();

    void clearMaskLayers() { m_rareNonInheritedData.access().mask = FillLayer(MaskFillLayer); }
    void inheritMaskLayers(const FillLayer& parent) { m_rareNonInheritedData.access().mask = parent; }

    void adjustMaskLayers();

    void setMaskImage(RefPtr<StyleImage>&& v) { m_rareNonInheritedData.access().mask.setImage(WTFMove(v)); }

    void setMaskBoxImage(const NinePieceImage& b) { SET_VAR(m_rareNonInheritedData, maskBoxImage, b); }
    void setMaskBoxImageSource(RefPtr<StyleImage>&& v) { m_rareNonInheritedData.access().maskBoxImage.setImage(WTFMove(v)); }
    void setMaskXPosition(Length&& length) { SET_VAR(m_rareNonInheritedData, mask.m_xPosition, WTFMove(length)); }
    void setMaskYPosition(Length&& length) { SET_VAR(m_rareNonInheritedData, mask.m_yPosition, WTFMove(length)); }
    void setMaskSize(LengthSize size) { SET_VAR(m_rareNonInheritedData, mask.m_sizeLength, WTFMove(size)); }

    void setBorderCollapse(EBorderCollapse collapse) { m_inheritedFlags.borderCollapse = collapse; }
    void setHorizontalBorderSpacing(float);
    void setVerticalBorderSpacing(float);
    void setEmptyCells(EEmptyCell v) { m_inheritedFlags.emptyCells = v; }
    void setCaptionSide(ECaptionSide v) { m_inheritedFlags.captionSide = v; }

    void setAspectRatioType(AspectRatioType aspectRatioType) { SET_VAR(m_rareNonInheritedData, aspectRatioType, aspectRatioType); }
    void setAspectRatioDenominator(float v) { SET_VAR(m_rareNonInheritedData, aspectRatioDenominator, v); }
    void setAspectRatioNumerator(float v) { SET_VAR(m_rareNonInheritedData, aspectRatioNumerator, v); }

    void setListStyleType(EListStyleType v) { m_inheritedFlags.listStyleType = v; }
    void setListStyleImage(RefPtr<StyleImage>&&);
    void setListStylePosition(EListStylePosition v) { m_inheritedFlags.listStylePosition = v; }

    void resetMargin() { SET_VAR(m_surroundData, margin, LengthBox(Fixed)); }
    void setMarginTop(Length&& length) { SET_VAR(m_surroundData, margin.top(), WTFMove(length)); }
    void setMarginBottom(Length&& length) { SET_VAR(m_surroundData, margin.bottom(), WTFMove(length)); }
    void setMarginLeft(Length&& length) { SET_VAR(m_surroundData, margin.left(), WTFMove(length)); }
    void setMarginRight(Length&& length) { SET_VAR(m_surroundData, margin.right(), WTFMove(length)); }
    void setMarginStart(Length&&);
    void setMarginEnd(Length&&);

    void resetPadding() { SET_VAR(m_surroundData, padding, LengthBox(Auto)); }
    void setPaddingBox(LengthBox&& box) { SET_VAR(m_surroundData, padding, WTFMove(box)); }
    void setPaddingTop(Length&& length) { SET_VAR(m_surroundData, padding.top(), WTFMove(length)); }
    void setPaddingBottom(Length&& length) { SET_VAR(m_surroundData, padding.bottom(), WTFMove(length)); }
    void setPaddingLeft(Length&& length) { SET_VAR(m_surroundData, padding.left(), WTFMove(length)); }
    void setPaddingRight(Length&& length) { SET_VAR(m_surroundData, padding.right(), WTFMove(length)); }

    void setCursor(ECursor c) { m_inheritedFlags.cursor = c; }
    void addCursor(RefPtr<StyleImage>&&, const IntPoint& hotSpot = IntPoint());
    void setCursorList(RefPtr<CursorList>&&);
    void clearCursorList();

#if ENABLE(CURSOR_VISIBILITY)
    void setCursorVisibility(CursorVisibility c) { m_inheritedFlags.cursorVisibility = c; }
#endif

    void setInsideLink(EInsideLink insideLink) { m_inheritedFlags.insideLink = insideLink; }
    void setIsLink(bool b) { m_nonInheritedFlags.setIsLink(b); }

    void setInsideDefaultButton(bool insideDefaultButton) { m_inheritedFlags.insideDefaultButton = insideDefaultButton; }

    PrintColorAdjust printColorAdjust() const { return static_cast<PrintColorAdjust>(m_inheritedFlags.printColorAdjust); }
    void setPrintColorAdjust(PrintColorAdjust value) { m_inheritedFlags.printColorAdjust = value; }

    bool hasAutoZIndex() const { return m_boxData->hasAutoZIndex(); }
    void setHasAutoZIndex() { SET_VAR(m_boxData, m_hasAutoZIndex, true); SET_VAR(m_boxData, m_zIndex, 0); }
    int zIndex() const { return m_boxData->zIndex(); }
    void setZIndex(int v) { SET_VAR(m_boxData, m_hasAutoZIndex, false); SET_VAR(m_boxData, m_zIndex, v); }

    void setHasAutoWidows() { SET_VAR(m_rareInheritedData, hasAutoWidows, true); SET_VAR(m_rareInheritedData, widows, initialWidows()); }
    void setWidows(short w) { SET_VAR(m_rareInheritedData, hasAutoWidows, false); SET_VAR(m_rareInheritedData, widows, w); }

    void setHasAutoOrphans() { SET_VAR(m_rareInheritedData, hasAutoOrphans, true); SET_VAR(m_rareInheritedData, orphans, initialOrphans()); }
    void setOrphans(short o) { SET_VAR(m_rareInheritedData, hasAutoOrphans, false); SET_VAR(m_rareInheritedData, orphans, o); }

    // CSS3 Setters
    void setOutlineOffset(float v) { SET_VAR(m_backgroundData, outline.m_offset, v); }
    void setTextShadow(std::unique_ptr<ShadowData>, bool add = false);
    void setTextStrokeColor(const Color& c) { SET_VAR(m_rareInheritedData, textStrokeColor, c); }
    void setTextStrokeWidth(float w) { SET_VAR(m_rareInheritedData, textStrokeWidth, w); }
    void setTextFillColor(const Color& c) { SET_VAR(m_rareInheritedData, textFillColor, c); }
    void setOpacity(float f) { float v = clampTo<float>(f, 0, 1); SET_VAR(m_rareNonInheritedData, opacity, v); }
    void setAppearance(ControlPart a) { SET_VAR(m_rareNonInheritedData, appearance, a); }
    // For valid values of box-align see http://www.w3.org/TR/2009/WD-css3-flexbox-20090723/#alignment
    void setBoxAlign(EBoxAlignment a) { SET_NESTED_VAR(m_rareNonInheritedData, deprecatedFlexibleBox, align, a); }
    void setBoxDirection(EBoxDirection d) { m_inheritedFlags.boxDirection = d; }
    void setBoxFlex(float f) { SET_NESTED_VAR(m_rareNonInheritedData, deprecatedFlexibleBox, flex, f); }
    void setBoxFlexGroup(unsigned group) { SET_NESTED_VAR(m_rareNonInheritedData, deprecatedFlexibleBox, flexGroup, group); }
    void setBoxLines(EBoxLines lines) { SET_NESTED_VAR(m_rareNonInheritedData, deprecatedFlexibleBox, lines, lines); }
    void setBoxOrdinalGroup(unsigned group) { SET_NESTED_VAR(m_rareNonInheritedData, deprecatedFlexibleBox, ordinalGroup, group); }
    void setBoxOrient(EBoxOrient o) { SET_NESTED_VAR(m_rareNonInheritedData, deprecatedFlexibleBox, orient, o); }
    void setBoxPack(EBoxPack p) { SET_NESTED_VAR(m_rareNonInheritedData, deprecatedFlexibleBox, pack, p); }
    void setBoxShadow(std::unique_ptr<ShadowData>, bool add = false);
    void setBoxReflect(RefPtr<StyleReflection>&&);
    void setBoxSizing(EBoxSizing s) { SET_VAR(m_boxData, m_boxSizing, s); }
    void setFlexGrow(float f) { SET_NESTED_VAR(m_rareNonInheritedData, flexibleBox, flexGrow, f); }
    void setFlexShrink(float f) { SET_NESTED_VAR(m_rareNonInheritedData, flexibleBox, flexShrink, f); }
    void setFlexBasis(Length&& length) { SET_NESTED_VAR(m_rareNonInheritedData, flexibleBox, flexBasis, WTFMove(length)); }
    void setOrder(int o) { SET_VAR(m_rareNonInheritedData, order, o); }
    void setAlignContent(const StyleContentAlignmentData& data) { SET_VAR(m_rareNonInheritedData, alignContent, data); }
    void setAlignItems(const StyleSelfAlignmentData& data) { SET_VAR(m_rareNonInheritedData, alignItems, data); }
    void setAlignItemsPosition(ItemPosition position) { m_rareNonInheritedData.access().alignItems.setPosition(position); }
    void setAlignSelf(const StyleSelfAlignmentData& data) { SET_VAR(m_rareNonInheritedData, alignSelf, data); }
    void setAlignSelfPosition(ItemPosition position) { m_rareNonInheritedData.access().alignSelf.setPosition(position); }
    void setFlexDirection(EFlexDirection direction) { SET_NESTED_VAR(m_rareNonInheritedData, flexibleBox, flexDirection, direction); }
    void setFlexWrap(EFlexWrap w) { SET_NESTED_VAR(m_rareNonInheritedData, flexibleBox, flexWrap, w); }
    void setJustifyContent(const StyleContentAlignmentData& data) { SET_VAR(m_rareNonInheritedData, justifyContent, data); }
    void setJustifyContentPosition(ContentPosition position) { m_rareNonInheritedData.access().justifyContent.setPosition(position); }
    void setJustifyItems(const StyleSelfAlignmentData& data) { SET_VAR(m_rareNonInheritedData, justifyItems, data); }
    void setJustifySelf(const StyleSelfAlignmentData& data) { SET_VAR(m_rareNonInheritedData, justifySelf, data); }

#if ENABLE(CSS_BOX_DECORATION_BREAK)
    void setBoxDecorationBreak(EBoxDecorationBreak b) { SET_VAR(m_boxData, m_boxDecorationBreak, b); }
#endif

#if ENABLE(CSS_GRID_LAYOUT)
    void setGridAutoColumns(const Vector<GridTrackSize>& trackSizeList) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridAutoColumns, trackSizeList); }
    void setGridAutoRows(const Vector<GridTrackSize>& trackSizeList) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridAutoRows, trackSizeList); }
    void setGridColumns(const Vector<GridTrackSize>& lengths) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridColumns, lengths); }
    void setGridRows(const Vector<GridTrackSize>& lengths) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridRows, lengths); }
    void setGridAutoRepeatColumns(const Vector<GridTrackSize>& lengths) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridAutoRepeatColumns, lengths); }
    void setGridAutoRepeatRows(const Vector<GridTrackSize>& lengths) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridAutoRepeatRows, lengths); }
    void setGridAutoRepeatColumnsInsertionPoint(const unsigned insertionPoint) { SET_NESTED_VAR(m_rareNonInheritedData, grid, autoRepeatColumnsInsertionPoint, insertionPoint); }
    void setGridAutoRepeatRowsInsertionPoint(const unsigned insertionPoint) { SET_NESTED_VAR(m_rareNonInheritedData, grid, autoRepeatRowsInsertionPoint, insertionPoint); }
    void setGridAutoRepeatColumnsType(const AutoRepeatType autoRepeatType) { SET_NESTED_VAR(m_rareNonInheritedData, grid, autoRepeatColumnsType, autoRepeatType); }
    void setGridAutoRepeatRowsType(const AutoRepeatType autoRepeatType) { SET_NESTED_VAR(m_rareNonInheritedData, grid, autoRepeatRowsType, autoRepeatType); }
    void setNamedGridColumnLines(const NamedGridLinesMap& namedGridColumnLines) { SET_NESTED_VAR(m_rareNonInheritedData, grid, namedGridColumnLines, namedGridColumnLines); }
    void setNamedGridRowLines(const NamedGridLinesMap& namedGridRowLines) { SET_NESTED_VAR(m_rareNonInheritedData, grid, namedGridRowLines, namedGridRowLines); }
    void setOrderedNamedGridColumnLines(const OrderedNamedGridLinesMap& orderedNamedGridColumnLines) { SET_NESTED_VAR(m_rareNonInheritedData, grid, orderedNamedGridColumnLines, orderedNamedGridColumnLines); }
    void setOrderedNamedGridRowLines(const OrderedNamedGridLinesMap& orderedNamedGridRowLines) { SET_NESTED_VAR(m_rareNonInheritedData, grid, orderedNamedGridRowLines, orderedNamedGridRowLines); }
    void setAutoRepeatNamedGridColumnLines(const NamedGridLinesMap& namedGridColumnLines) { SET_NESTED_VAR(m_rareNonInheritedData, grid, autoRepeatNamedGridColumnLines, namedGridColumnLines); }
    void setAutoRepeatNamedGridRowLines(const NamedGridLinesMap& namedGridRowLines) { SET_NESTED_VAR(m_rareNonInheritedData, grid, autoRepeatNamedGridRowLines, namedGridRowLines); }
    void setAutoRepeatOrderedNamedGridColumnLines(const OrderedNamedGridLinesMap& orderedNamedGridColumnLines) { SET_NESTED_VAR(m_rareNonInheritedData, grid, autoRepeatOrderedNamedGridColumnLines, orderedNamedGridColumnLines); }
    void setAutoRepeatOrderedNamedGridRowLines(const OrderedNamedGridLinesMap& orderedNamedGridRowLines) { SET_NESTED_VAR(m_rareNonInheritedData, grid, autoRepeatOrderedNamedGridRowLines, orderedNamedGridRowLines); }
    void setNamedGridArea(const NamedGridAreaMap& namedGridArea) { SET_NESTED_VAR(m_rareNonInheritedData, grid, namedGridArea, namedGridArea); }
    void setNamedGridAreaRowCount(size_t rowCount) { SET_NESTED_VAR(m_rareNonInheritedData, grid, namedGridAreaRowCount, rowCount); }
    void setNamedGridAreaColumnCount(size_t columnCount) { SET_NESTED_VAR(m_rareNonInheritedData, grid, namedGridAreaColumnCount, columnCount); }
    void setGridAutoFlow(GridAutoFlow flow) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridAutoFlow, flow); }
    void setGridItemColumnStart(const GridPosition& columnStartPosition) { SET_NESTED_VAR(m_rareNonInheritedData, gridItem, gridColumnStart, columnStartPosition); }
    void setGridItemColumnEnd(const GridPosition& columnEndPosition) { SET_NESTED_VAR(m_rareNonInheritedData, gridItem, gridColumnEnd, columnEndPosition); }
    void setGridItemRowStart(const GridPosition& rowStartPosition) { SET_NESTED_VAR(m_rareNonInheritedData, gridItem, gridRowStart, rowStartPosition); }
    void setGridItemRowEnd(const GridPosition& rowEndPosition) { SET_NESTED_VAR(m_rareNonInheritedData, gridItem, gridRowEnd, rowEndPosition); }
    void setGridColumnGap(Length&& length) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridColumnGap, WTFMove(length)); }
    void setGridRowGap(Length&& length) { SET_NESTED_VAR(m_rareNonInheritedData, grid, gridRowGap, WTFMove(length)); }
#endif // ENABLE(CSS_GRID_LAYOUT)

    void setMarqueeIncrement(Length&& length) { SET_NESTED_VAR(m_rareNonInheritedData, marquee, increment, WTFMove(length)); }
    void setMarqueeSpeed(int f) { SET_NESTED_VAR(m_rareNonInheritedData, marquee, speed, f); }
    void setMarqueeDirection(EMarqueeDirection d) { SET_NESTED_VAR(m_rareNonInheritedData, marquee, direction, d); }
    void setMarqueeBehavior(EMarqueeBehavior b) { SET_NESTED_VAR(m_rareNonInheritedData, marquee, behavior, b); }
    void setMarqueeLoopCount(int i) { SET_NESTED_VAR(m_rareNonInheritedData, marquee, loops, i); }
    void setUserModify(EUserModify u) { SET_VAR(m_rareInheritedData, userModify, u); }
    void setUserDrag(EUserDrag d) { SET_VAR(m_rareNonInheritedData, userDrag, d); }
    void setUserSelect(EUserSelect s) { SET_VAR(m_rareInheritedData, userSelect, s); }
    void setTextOverflow(TextOverflow overflow) { SET_VAR(m_rareNonInheritedData, textOverflow, overflow); }
    void setMarginBeforeCollapse(EMarginCollapse c) { SET_VAR(m_rareNonInheritedData, marginBeforeCollapse, c); }
    void setMarginAfterCollapse(EMarginCollapse c) { SET_VAR(m_rareNonInheritedData, marginAfterCollapse, c); }
    void setWordBreak(EWordBreak b) { SET_VAR(m_rareInheritedData, wordBreak, b); }
    void setOverflowWrap(EOverflowWrap b) { SET_VAR(m_rareInheritedData, overflowWrap, b); }
    void setNBSPMode(ENBSPMode b) { SET_VAR(m_rareInheritedData, nbspMode, b); }
    void setLineBreak(LineBreak b) { SET_VAR(m_rareInheritedData, lineBreak, b); }
    void setHyphens(Hyphens h) { SET_VAR(m_rareInheritedData, hyphens, h); }
    void setHyphenationLimitBefore(short limit) { SET_VAR(m_rareInheritedData, hyphenationLimitBefore, limit); }
    void setHyphenationLimitAfter(short limit) { SET_VAR(m_rareInheritedData, hyphenationLimitAfter, limit); }
    void setHyphenationLimitLines(short limit) { SET_VAR(m_rareInheritedData, hyphenationLimitLines, limit); }
    void setHyphenationString(const AtomicString& h) { SET_VAR(m_rareInheritedData, hyphenationString, h); }
    void setBorderFit(EBorderFit b) { SET_VAR(m_rareNonInheritedData, borderFit, b); }
    void setResize(EResize r) { SET_VAR(m_rareNonInheritedData, resize, r); }
    void setColumnAxis(ColumnAxis axis) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, axis, axis); }
    void setColumnProgression(ColumnProgression progression) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, progression, progression); }
    void setColumnWidth(float f) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, autoWidth, false); SET_NESTED_VAR(m_rareNonInheritedData, multiCol, width, f); }
    void setHasAutoColumnWidth() { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, autoWidth, true); SET_NESTED_VAR(m_rareNonInheritedData, multiCol, width, 0); }
    void setColumnCount(unsigned short c) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, autoCount, false); SET_NESTED_VAR(m_rareNonInheritedData, multiCol, count, c); }
    void setHasAutoColumnCount() { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, autoCount, true); SET_NESTED_VAR(m_rareNonInheritedData, multiCol, count, 0); }
    void setColumnFill(ColumnFill columnFill) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, fill, columnFill); }
    void setColumnGap(float f) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, normalGap, false); SET_NESTED_VAR(m_rareNonInheritedData, multiCol, gap, f); }
    void setHasNormalColumnGap() { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, normalGap, true); SET_NESTED_VAR(m_rareNonInheritedData, multiCol, gap, 0); }
    void setColumnRuleColor(const Color& c) { SET_BORDERVALUE_COLOR(m_rareNonInheritedData.access().multiCol, rule, c); }
    void setColumnRuleStyle(EBorderStyle b) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, rule.m_style, b); }
    void setColumnRuleWidth(unsigned short w) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, rule.m_width, w); }
    void resetColumnRule() { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, rule, BorderValue()); }
    void setColumnSpan(ColumnSpan columnSpan) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, columnSpan, columnSpan); }
    void inheritColumnPropertiesFrom(const RenderStyle& parent) { m_rareNonInheritedData.access().multiCol = parent.m_rareNonInheritedData->multiCol; }
    void setTransform(const TransformOperations& ops) { SET_NESTED_VAR(m_rareNonInheritedData, transform, operations, ops); }
    void setTransformOriginX(Length&& length) { SET_NESTED_VAR(m_rareNonInheritedData, transform, x, WTFMove(length)); }
    void setTransformOriginY(Length&& length) { SET_NESTED_VAR(m_rareNonInheritedData, transform, y, WTFMove(length)); }
    void setTransformOriginZ(float f) { SET_NESTED_VAR(m_rareNonInheritedData, transform, z, f); }
    void setSpeak(ESpeak s) { SET_VAR(m_rareInheritedData, speak, s); }
    void setTextCombine(TextCombine v) { SET_VAR(m_rareNonInheritedData, textCombine, v); }
    void setTextDecorationColor(const Color& c) { SET_VAR(m_rareNonInheritedData, textDecorationColor, c); }
    void setTextEmphasisColor(const Color& c) { SET_VAR(m_rareInheritedData, textEmphasisColor, c); }
    void setTextEmphasisFill(TextEmphasisFill fill) { SET_VAR(m_rareInheritedData, textEmphasisFill, fill); }
    void setTextEmphasisMark(TextEmphasisMark mark) { SET_VAR(m_rareInheritedData, textEmphasisMark, mark); }
    void setTextEmphasisCustomMark(const AtomicString& mark) { SET_VAR(m_rareInheritedData, textEmphasisCustomMark, mark); }
    void setTextEmphasisPosition(TextEmphasisPosition position) { SET_VAR(m_rareInheritedData, textEmphasisPosition, position); }
    bool setTextOrientation(TextOrientation);

    void setObjectFit(ObjectFit fit) { SET_VAR(m_rareNonInheritedData, objectFit, fit); }
    void setObjectPosition(LengthPoint&& position) { SET_VAR(m_rareNonInheritedData, objectPosition, WTFMove(position)); }

    void setRubyPosition(RubyPosition position) { SET_VAR(m_rareInheritedData, rubyPosition, position); }

    void setFilter(const FilterOperations& ops) { SET_NESTED_VAR(m_rareNonInheritedData, filter, operations, ops); }

#if ENABLE(FILTERS_LEVEL_2)
    void setBackdropFilter(const FilterOperations& ops) { SET_NESTED_VAR(m_rareNonInheritedData, backdropFilter, operations, ops); }
#endif

    void setTabSize(unsigned size) { SET_VAR(m_rareInheritedData, tabSize, size); }

    void setBreakBefore(BreakBetween breakBehavior) { SET_VAR(m_rareNonInheritedData, breakBefore, breakBehavior); }
    void setBreakAfter(BreakBetween breakBehavior) { SET_VAR(m_rareNonInheritedData, breakAfter, breakBehavior); }
    void setBreakInside(BreakInside breakBehavior) { SET_VAR(m_rareNonInheritedData, breakInside, breakBehavior); }
    
    void setHangingPunctuation(HangingPunctuation punctuation) { SET_VAR(m_rareInheritedData, hangingPunctuation, punctuation); }

    // End CSS3 Setters

    void setLineGrid(const AtomicString& lineGrid) { SET_VAR(m_rareInheritedData, lineGrid, lineGrid); }
    void setLineSnap(LineSnap lineSnap) { SET_VAR(m_rareInheritedData, lineSnap, lineSnap); }
    void setLineAlign(LineAlign lineAlign) { SET_VAR(m_rareInheritedData, lineAlign, lineAlign); }

    void setFlowThread(const AtomicString& flowThread) { SET_VAR(m_rareNonInheritedData, flowThread, flowThread); }
    void setRegionThread(const AtomicString& regionThread) { SET_VAR(m_rareNonInheritedData, regionThread, regionThread); }
    void setRegionFragment(RegionFragment regionFragment) { SET_VAR(m_rareNonInheritedData, regionFragment, regionFragment); }

    void setPointerEvents(EPointerEvents p) { m_inheritedFlags.pointerEvents = p; }

    void clearAnimations();
    void clearTransitions();

    void adjustAnimations();
    void adjustTransitions();

    void setTransformStyle3D(ETransformStyle3D b) { SET_VAR(m_rareNonInheritedData, transformStyle3D, b); }
    void setBackfaceVisibility(EBackfaceVisibility b) { SET_VAR(m_rareNonInheritedData, backfaceVisibility, b); }
    void setPerspective(float p) { SET_VAR(m_rareNonInheritedData, perspective, p); }
    void setPerspectiveOriginX(Length&& length) { SET_VAR(m_rareNonInheritedData, perspectiveOriginX, WTFMove(length)); }
    void setPerspectiveOriginY(Length&& length) { SET_VAR(m_rareNonInheritedData, perspectiveOriginY, WTFMove(length)); }
    void setPageSize(LengthSize size) { SET_VAR(m_rareNonInheritedData, pageSize, WTFMove(size)); }
    void setPageSizeType(PageSizeType t) { SET_VAR(m_rareNonInheritedData, pageSizeType, t); }
    void resetPageSizeType() { SET_VAR(m_rareNonInheritedData, pageSizeType, PAGE_SIZE_AUTO); }

    void setLineBoxContain(LineBoxContain c) { SET_VAR(m_rareInheritedData, lineBoxContain, c); }
    void setLineClamp(LineClampValue c) { SET_VAR(m_rareNonInheritedData, lineClamp, c); }
    
    void setInitialLetter(const IntSize& size) { SET_VAR(m_rareNonInheritedData, initialLetter, size); }
    
#if ENABLE(TOUCH_EVENTS)
    void setTouchAction(TouchAction touchAction) { SET_VAR(m_rareNonInheritedData, touchAction, static_cast<unsigned>(touchAction)); }
#endif

#if ENABLE(CSS_SCROLL_SNAP)
    void setScrollSnapType(const ScrollSnapType&);
    void setScrollPaddingTop(Length&&);
    void setScrollPaddingBottom(Length&&);
    void setScrollPaddingLeft(Length&&);
    void setScrollPaddingRight(Length&&);

    void setScrollSnapAlign(const ScrollSnapAlign&);
    void setScrollSnapMarginTop(Length&&);
    void setScrollSnapMarginBottom(Length&&);
    void setScrollSnapMarginLeft(Length&&);
    void setScrollSnapMarginRight(Length&&);
#endif

#if ENABLE(TOUCH_EVENTS)
    void setTapHighlightColor(const Color& c) { SET_VAR(m_rareInheritedData, tapHighlightColor, c); }
#endif

#if PLATFORM(IOS)
    void setTouchCalloutEnabled(bool v) { SET_VAR(m_rareInheritedData, touchCalloutEnabled, v); }
#endif

#if ENABLE(ACCELERATED_OVERFLOW_SCROLLING)
    void setUseTouchOverflowScrolling(bool v) { SET_VAR(m_rareInheritedData, useTouchOverflowScrolling, v); }
#endif

#if ENABLE(TEXT_AUTOSIZING)
    void setTextSizeAdjust(TextSizeAdjustment adjustment) { SET_VAR(m_rareInheritedData, textSizeAdjust, adjustment); }
#endif

    void setTextSecurity(ETextSecurity security) { SET_VAR(m_rareInheritedData, textSecurity, security); }

#if ENABLE(CSS_TRAILING_WORD)
    void setTrailingWord(TrailingWord v) { SET_VAR(m_rareInheritedData, trailingWord, static_cast<unsigned>(v)); }
#endif

#if ENABLE(APPLE_PAY)
    void setApplePayButtonStyle(ApplePayButtonStyle style) { SET_VAR(m_rareNonInheritedData, applePayButtonStyle, static_cast<unsigned>(style)); }
    void setApplePayButtonType(ApplePayButtonType type) { SET_VAR(m_rareNonInheritedData, applePayButtonType, static_cast<unsigned>(type)); }
#endif

    const SVGRenderStyle& svgStyle() const { return m_svgStyle; }
    SVGRenderStyle& accessSVGStyle() { return m_svgStyle.access(); }

    const SVGPaintType& fillPaintType() const { return svgStyle().fillPaintType(); }
    Color fillPaintColor() const { return svgStyle().fillPaintColor(); }
    void setFillPaintColor(const Color& color) { accessSVGStyle().setFillPaint(SVG_PAINTTYPE_RGBCOLOR, color, emptyString()); }
    float fillOpacity() const { return svgStyle().fillOpacity(); }
    void setFillOpacity(float f) { accessSVGStyle().setFillOpacity(f); }

    const SVGPaintType& strokePaintType() const { return svgStyle().strokePaintType(); }
    Color strokePaintColor() const { return svgStyle().strokePaintColor(); }
    void setStrokePaintColor(const Color& color) { accessSVGStyle().setStrokePaint(SVG_PAINTTYPE_RGBCOLOR, color, emptyString()); }
    float strokeOpacity() const { return svgStyle().strokeOpacity(); }
    void setStrokeOpacity(float f) { accessSVGStyle().setStrokeOpacity(f); }
    const Length& strokeWidth() const { return svgStyle().strokeWidth(); }
    void setStrokeWidth(Length&& w) { accessSVGStyle().setStrokeWidth(WTFMove(w)); }
    Vector<SVGLengthValue> strokeDashArray() const { return svgStyle().strokeDashArray(); }
    void setStrokeDashArray(Vector<SVGLengthValue> array) { accessSVGStyle().setStrokeDashArray(array); }
    const Length& strokeDashOffset() const { return svgStyle().strokeDashOffset(); }
    void setStrokeDashOffset(Length&& d) { accessSVGStyle().setStrokeDashOffset(WTFMove(d)); }
    float strokeMiterLimit() const { return svgStyle().strokeMiterLimit(); }
    void setStrokeMiterLimit(float f) { accessSVGStyle().setStrokeMiterLimit(f); }

    const Length& cx() const { return svgStyle().cx(); }
    void setCx(Length&& cx) { accessSVGStyle().setCx(WTFMove(cx)); }
    const Length& cy() const { return svgStyle().cy(); }
    void setCy(Length&& cy) { accessSVGStyle().setCy(WTFMove(cy)); }
    const Length& r() const { return svgStyle().r(); }
    void setR(Length&& r) { accessSVGStyle().setR(WTFMove(r)); }
    const Length& rx() const { return svgStyle().rx(); }
    void setRx(Length&& rx) { accessSVGStyle().setRx(WTFMove(rx)); }
    const Length& ry() const { return svgStyle().ry(); }
    void setRy(Length&& ry) { accessSVGStyle().setRy(WTFMove(ry)); }
    const Length& x() const { return svgStyle().x(); }
    void setX(Length&& x) { accessSVGStyle().setX(WTFMove(x)); }
    const Length& y() const { return svgStyle().y(); }
    void setY(Length&& y) { accessSVGStyle().setY(WTFMove(y)); }

    float floodOpacity() const { return svgStyle().floodOpacity(); }
    void setFloodOpacity(float f) { accessSVGStyle().setFloodOpacity(f); }

    float stopOpacity() const { return svgStyle().stopOpacity(); }
    void setStopOpacity(float f) { accessSVGStyle().setStopOpacity(f); }

    void setStopColor(const Color& c) { accessSVGStyle().setStopColor(c); }
    void setFloodColor(const Color& c) { accessSVGStyle().setFloodColor(c); }
    void setLightingColor(const Color& c) { accessSVGStyle().setLightingColor(c); }

    SVGLengthValue baselineShiftValue() const { return svgStyle().baselineShiftValue(); }
    void setBaselineShiftValue(SVGLengthValue s) { accessSVGStyle().setBaselineShiftValue(s); }
    SVGLengthValue kerning() const { return svgStyle().kerning(); }
    void setKerning(SVGLengthValue k) { accessSVGStyle().setKerning(k); }

    void setShapeOutside(RefPtr<ShapeValue>&&);
    ShapeValue* shapeOutside() const { return m_rareNonInheritedData->shapeOutside.get(); }
    static ShapeValue* initialShapeOutside() { return nullptr; }

    const Length& shapeMargin() const { return m_rareNonInheritedData->shapeMargin; }
    void setShapeMargin(Length&& shapeMargin) { SET_VAR(m_rareNonInheritedData, shapeMargin, WTFMove(shapeMargin)); }
    static Length initialShapeMargin() { return Length(0, Fixed); }

    float shapeImageThreshold() const { return m_rareNonInheritedData->shapeImageThreshold; }
    void setShapeImageThreshold(float);
    static float initialShapeImageThreshold() { return 0; }

    void setClipPath(RefPtr<ClipPathOperation>&&);
    ClipPathOperation* clipPath() const { return m_rareNonInheritedData->clipPath.get(); }
    static ClipPathOperation* initialClipPath() { return nullptr; }

    bool hasContent() const { return contentData(); }
    const ContentData* contentData() const { return m_rareNonInheritedData->content.get(); }
    bool contentDataEquivalent(const RenderStyle* otherStyle) const { return const_cast<RenderStyle*>(this)->m_rareNonInheritedData->contentDataEquivalent(*const_cast<RenderStyle*>(otherStyle)->m_rareNonInheritedData); }
    void clearContent();
    void setContent(const String&, bool add = false);
    void setContent(RefPtr<StyleImage>&&, bool add = false);
    void setContent(std::unique_ptr<CounterContent>, bool add = false);
    void setContent(QuoteType, bool add = false);
    void setContentAltText(const String&);
    const String& contentAltText() const;
    bool hasAttrContent() const { return m_rareNonInheritedData->hasAttrContent; }
    void setHasAttrContent();

    const CounterDirectiveMap* counterDirectives() const;
    CounterDirectiveMap& accessCounterDirectives();
    const CounterDirectives getCounterDirectives(const AtomicString& identifier) const;

    QuotesData* quotes() const { return m_rareInheritedData->quotes.get(); }
    void setQuotes(RefPtr<QuotesData>&&);

    WillChangeData* willChange() const { return m_rareNonInheritedData->willChange.get(); }
    void setWillChange(RefPtr<WillChangeData>&&);

    bool willChangeCreatesStackingContext() const;

    const AtomicString& hyphenString() const;

    bool inheritedNotEqual(const RenderStyle*) const;
    bool inheritedDataShared(const RenderStyle*) const;

#if ENABLE(TEXT_AUTOSIZING)
    uint32_t hashForTextAutosizing() const;
    bool equalForTextAutosizing(const RenderStyle&) const;
#endif

    StyleDifference diff(const RenderStyle&, unsigned& changedContextSensitiveProperties) const;
    bool diffRequiresLayerRepaint(const RenderStyle&, bool isComposited) const;

    bool isDisplayInlineType() const { return isDisplayInlineType(display()); }
    bool isOriginalDisplayInlineType() const { return isDisplayInlineType(originalDisplay()); }
    bool isDisplayFlexibleOrGridBox() const { return isDisplayFlexibleOrGridBox(display()); }
    bool isDisplayRegionType() const;

    bool setWritingMode(WritingMode);

    bool hasExplicitlySetWritingMode() const { return m_nonInheritedFlags.hasExplicitlySetWritingMode(); }
    void setHasExplicitlySetWritingMode(bool v) { m_nonInheritedFlags.setHasExplicitlySetWritingMode(v); }

    // A unique style is one that has matches something that makes it impossible to share.
    bool unique() const { return m_nonInheritedFlags.isUnique(); }
    void setUnique() { m_nonInheritedFlags.setIsUnique(); }

    bool emptyState() const { return m_nonInheritedFlags.emptyState(); }
    void setEmptyState(bool b) { setUnique(); m_nonInheritedFlags.setEmptyState(b); }
    bool firstChildState() const { return m_nonInheritedFlags.firstChildState(); }
    void setFirstChildState() { setUnique(); m_nonInheritedFlags.setFirstChildState(true); }
    bool lastChildState() const { return m_nonInheritedFlags.lastChildState(); }
    void setLastChildState() { setUnique(); m_nonInheritedFlags.setLastChildState(true); }

    WEBCORE_EXPORT Color visitedDependentColor(int colorProperty) const;
    bool backgroundColorEqualsToColorIgnoringVisited(const Color& color) const { return color == backgroundColor(); }

    void setHasExplicitlyInheritedProperties() { m_nonInheritedFlags.setHasExplicitlyInheritedProperties(true); }
    bool hasExplicitlyInheritedProperties() const { return m_nonInheritedFlags.hasExplicitlyInheritedProperties(); }
    
    // Initial values for all the properties
    static EOverflow initialOverflowX() { return OVISIBLE; }
    static EOverflow initialOverflowY() { return OVISIBLE; }
    static EClear initialClear() { return CNONE; }
    static EDisplay initialDisplay() { return INLINE; }
    static EUnicodeBidi initialUnicodeBidi() { return UBNormal; }
    static EPosition initialPosition() { return StaticPosition; }
    static EVerticalAlign initialVerticalAlign() { return BASELINE; }
    static EFloat initialFloating() { return NoFloat; }
    static BreakBetween initialBreakBetween() { return AutoBreakBetween; }
    static BreakInside initialBreakInside() { return AutoBreakInside; }
    static HangingPunctuation initialHangingPunctuation() { return NoHangingPunctuation; }
    static ETableLayout initialTableLayout() { return TAUTO; }
    static EBorderCollapse initialBorderCollapse() { return BSEPARATE; }
    static EBorderStyle initialBorderStyle() { return BNONE; }
    static OutlineIsAuto initialOutlineStyleIsAuto() { return AUTO_OFF; }
    static NinePieceImage initialNinePieceImage() { return NinePieceImage(); }
    static LengthSize initialBorderRadius() { return { { 0, Fixed }, { 0, Fixed } }; }
    static ECaptionSide initialCaptionSide() { return CAPTOP; }
    static ColumnAxis initialColumnAxis() { return AutoColumnAxis; }
    static ColumnProgression initialColumnProgression() { return NormalColumnProgression; }
    static TextDirection initialDirection() { return LTR; }
    static WritingMode initialWritingMode() { return TopToBottomWritingMode; }
    static TextCombine initialTextCombine() { return TextCombineNone; }
    static TextOrientation initialTextOrientation() { return TextOrientation::Mixed; }
    static ObjectFit initialObjectFit() { return ObjectFitFill; }
    static LengthPoint initialObjectPosition() { return LengthPoint(Length(50.0f, Percent), Length(50.0f, Percent)); }
    static EEmptyCell initialEmptyCells() { return SHOW; }
    static EListStylePosition initialListStylePosition() { return OUTSIDE; }
    static EListStyleType initialListStyleType() { return Disc; }
    static ETextTransform initialTextTransform() { return TTNONE; }
    static EVisibility initialVisibility() { return VISIBLE; }
    static EWhiteSpace initialWhiteSpace() { return NORMAL; }
    static float initialHorizontalBorderSpacing() { return 0; }
    static float initialVerticalBorderSpacing() { return 0; }
    static ECursor initialCursor() { return CursorAuto; }
    static Color initialColor() { return Color::black; }
    static StyleImage* initialListStyleImage() { return 0; }
    static float initialBorderWidth() { return 3; }
    static unsigned short initialColumnRuleWidth() { return 3; }
    static float initialOutlineWidth() { return 3; }
    static float initialLetterSpacing() { return 0; }
    static Length initialWordSpacing() { return Length(Fixed); }
    static Length initialSize() { return Length(); }
    static Length initialMinSize() { return Length(); }
    static Length initialMaxSize() { return Length(Undefined); }
    static Length initialOffset() { return Length(); }
    static Length initialMargin() { return Length(Fixed); }
    static Length initialPadding() { return Length(Fixed); }
    static Length initialTextIndent() { return Length(Fixed); }
    static Length initialZeroLength() { return Length(Fixed); }
    static Length initialOneLength() { return Length(1, Fixed); }
    static short initialWidows() { return 2; }
    static short initialOrphans() { return 2; }
    static Length initialLineHeight() { return Length(-100.0f, Percent); }
    static ETextAlign initialTextAlign() { return TASTART; }
    static TextDecoration initialTextDecoration() { return TextDecorationNone; }
    static TextDecorationStyle initialTextDecorationStyle() { return TextDecorationStyleSolid; }
    static TextDecorationSkip initialTextDecorationSkip() { return TextDecorationSkipAuto; }
    static TextUnderlinePosition initialTextUnderlinePosition() { return TextUnderlinePositionAuto; }
    static float initialZoom() { return 1.0f; }
    static TextZoom initialTextZoom() { return TextZoomNormal; }
    static float initialOutlineOffset() { return 0; }
    static float initialOpacity() { return 1.0f; }
    static EBoxAlignment initialBoxAlign() { return BSTRETCH; }
    static EBoxDecorationBreak initialBoxDecorationBreak() { return DSLICE; }
    static EBoxDirection initialBoxDirection() { return BNORMAL; }
    static EBoxLines initialBoxLines() { return SINGLE; }
    static EBoxOrient initialBoxOrient() { return HORIZONTAL; }
    static EBoxPack initialBoxPack() { return Start; }
    static float initialBoxFlex() { return 0.0f; }
    static unsigned initialBoxFlexGroup() { return 1; }
    static unsigned initialBoxOrdinalGroup() { return 1; }
    static EBoxSizing initialBoxSizing() { return CONTENT_BOX; }
    static StyleReflection* initialBoxReflect() { return 0; }
    static float initialFlexGrow() { return 0; }
    static float initialFlexShrink() { return 1; }
    static Length initialFlexBasis() { return Length(Auto); }
    static int initialOrder() { return 0; }
    static StyleSelfAlignmentData initialSelfAlignment() { return StyleSelfAlignmentData(ItemPositionAuto, OverflowAlignmentDefault); }
    static StyleSelfAlignmentData initialDefaultAlignment() { return StyleSelfAlignmentData(isCSSGridLayoutEnabled() ? ItemPositionNormal : ItemPositionStretch, OverflowAlignmentDefault); }
    static StyleContentAlignmentData initialContentAlignment() { return StyleContentAlignmentData(ContentPositionNormal, ContentDistributionDefault, OverflowAlignmentDefault); }
    static EFlexDirection initialFlexDirection() { return FlowRow; }
    static EFlexWrap initialFlexWrap() { return FlexNoWrap; }
    static int initialMarqueeLoopCount() { return -1; }
    static int initialMarqueeSpeed() { return 85; }
    static Length initialMarqueeIncrement() { return Length(6, Fixed); }
    static EMarqueeBehavior initialMarqueeBehavior() { return MSCROLL; }
    static EMarqueeDirection initialMarqueeDirection() { return MAUTO; }
    static EUserModify initialUserModify() { return READ_ONLY; }
    static EUserDrag initialUserDrag() { return DRAG_AUTO; }
    static EUserSelect initialUserSelect() { return SELECT_TEXT; }
    static TextOverflow initialTextOverflow() { return TextOverflowClip; }
    static EMarginCollapse initialMarginBeforeCollapse() { return MCOLLAPSE; }
    static EMarginCollapse initialMarginAfterCollapse() { return MCOLLAPSE; }
    static EWordBreak initialWordBreak() { return NormalWordBreak; }
    static EOverflowWrap initialOverflowWrap() { return NormalOverflowWrap; }
    static ENBSPMode initialNBSPMode() { return NBNORMAL; }
    static LineBreak initialLineBreak() { return LineBreakAuto; }
    static ESpeak initialSpeak() { return SpeakNormal; }
    static Hyphens initialHyphens() { return HyphensManual; }
    static short initialHyphenationLimitBefore() { return -1; }
    static short initialHyphenationLimitAfter() { return -1; }
    static short initialHyphenationLimitLines() { return -1; }
    static const AtomicString& initialHyphenationString() { return nullAtom; }
    static EBorderFit initialBorderFit() { return BorderFitBorder; }
    static EResize initialResize() { return RESIZE_NONE; }
    static ControlPart initialAppearance() { return NoControlPart; }
    static AspectRatioType initialAspectRatioType() { return AspectRatioAuto; }
    static float initialAspectRatioDenominator() { return 1; }
    static float initialAspectRatioNumerator() { return 1; }
    static Order initialRTLOrdering() { return LogicalOrder; }
    static float initialTextStrokeWidth() { return 0; }
    static unsigned short initialColumnCount() { return 1; }
    static ColumnFill initialColumnFill() { return ColumnFillBalance; }
    static ColumnSpan initialColumnSpan() { return ColumnSpanNone; }
    static const TransformOperations& initialTransform() { static NeverDestroyed<TransformOperations> ops; return ops; }
    static Length initialTransformOriginX() { return Length(50.0f, Percent); }
    static Length initialTransformOriginY() { return Length(50.0f, Percent); }
    static EPointerEvents initialPointerEvents() { return PE_AUTO; }
    static float initialTransformOriginZ() { return 0; }
    static ETransformStyle3D initialTransformStyle3D() { return TransformStyle3DFlat; }
    static EBackfaceVisibility initialBackfaceVisibility() { return BackfaceVisibilityVisible; }
    static float initialPerspective() { return 0; }
    static Length initialPerspectiveOriginX() { return Length(50.0f, Percent); }
    static Length initialPerspectiveOriginY() { return Length(50.0f, Percent); }
    static Color initialBackgroundColor() { return Color::transparent; }
    static Color initialTextEmphasisColor() { return TextEmphasisFillFilled; }
    static TextEmphasisFill initialTextEmphasisFill() { return TextEmphasisFillFilled; }
    static TextEmphasisMark initialTextEmphasisMark() { return TextEmphasisMarkNone; }
    static const AtomicString& initialTextEmphasisCustomMark() { return nullAtom; }
    static TextEmphasisPosition initialTextEmphasisPosition() { return TextEmphasisPositionOver | TextEmphasisPositionRight; }
    static RubyPosition initialRubyPosition() { return RubyPositionBefore; }
    static LineBoxContain initialLineBoxContain() { return LineBoxContainBlock | LineBoxContainInline | LineBoxContainReplaced; }
    static ImageOrientationEnum initialImageOrientation() { return OriginTopLeft; }
    static EImageRendering initialImageRendering() { return ImageRenderingAuto; }
    static ImageResolutionSource initialImageResolutionSource() { return ImageResolutionSpecified; }
    static ImageResolutionSnap initialImageResolutionSnap() { return ImageResolutionNoSnap; }
    static float initialImageResolution() { return 1; }
    static StyleImage* initialBorderImageSource() { return nullptr; }
    static StyleImage* initialMaskBoxImageSource() { return nullptr; }
    static PrintColorAdjust initialPrintColorAdjust() { return PrintColorAdjustEconomy; }
    static QuotesData* initialQuotes() { return nullptr; }
    static const AtomicString& initialContentAltText() { return emptyAtom; }

#if ENABLE(CSS3_TEXT)
    static TextIndentLine initialTextIndentLine() { return TextIndentFirstLine; }
    static TextIndentType initialTextIndentType() { return TextIndentNormal; }
    static TextAlignLast initialTextAlignLast() { return TextAlignLastAuto; }
    static TextJustify initialTextJustify() { return TextJustifyAuto; }
#endif

#if ENABLE(CURSOR_VISIBILITY)
    static CursorVisibility initialCursorVisibility() { return CursorVisibilityAuto; }
#endif

#if ENABLE(TEXT_AUTOSIZING)
    static Length initialSpecifiedLineHeight() { return Length(-100.0f, Percent); }
    static TextSizeAdjustment initialTextSizeAdjust() { return TextSizeAdjustment(); }
#endif

    static bool isCSSGridLayoutEnabled();

    static WillChangeData* initialWillChange() { return nullptr; }

#if ENABLE(TOUCH_EVENTS)
    static TouchAction initialTouchAction() { return TouchAction::Auto; }
#endif

#if ENABLE(CSS_SCROLL_SNAP)
    static ScrollSnapType initialScrollSnapType();
    static ScrollSnapAlign initialScrollSnapAlign();
    static Length initialScrollSnapMargin() { return Length(Fixed); }
    static Length initialScrollPadding() { return Length(Fixed); }
#endif

#if ENABLE(CSS_TRAILING_WORD)
    static TrailingWord initialTrailingWord() { return TrailingWord::Auto; }
#endif

#if ENABLE(APPLE_PAY)
    static ApplePayButtonStyle initialApplePayButtonStyle() { return ApplePayButtonStyle::Black; }
    static ApplePayButtonType initialApplePayButtonType() { return ApplePayButtonType::Plain; }
#endif

#if ENABLE(CSS_GRID_LAYOUT)
    // The initial value is 'none' for grid tracks.
    static Vector<GridTrackSize> initialGridColumns() { return Vector<GridTrackSize>(); }
    static Vector<GridTrackSize> initialGridRows() { return Vector<GridTrackSize>(); }

    static Vector<GridTrackSize> initialGridAutoRepeatTracks() { return Vector<GridTrackSize>(); }
    static unsigned initialGridAutoRepeatInsertionPoint() { return 0; }
    static AutoRepeatType initialGridAutoRepeatType() { return NoAutoRepeat; }

    static GridAutoFlow initialGridAutoFlow() { return AutoFlowRow; }

    static Vector<GridTrackSize> initialGridAutoColumns() { return { GridTrackSize(Length(Auto)) }; }
    static Vector<GridTrackSize> initialGridAutoRows() { return { GridTrackSize(Length(Auto)) }; }

    static NamedGridAreaMap initialNamedGridArea() { return NamedGridAreaMap(); }
    static size_t initialNamedGridAreaCount() { return 0; }

    static NamedGridLinesMap initialNamedGridColumnLines() { return NamedGridLinesMap(); }
    static NamedGridLinesMap initialNamedGridRowLines() { return NamedGridLinesMap(); }

    static OrderedNamedGridLinesMap initialOrderedNamedGridColumnLines() { return OrderedNamedGridLinesMap(); }
    static OrderedNamedGridLinesMap initialOrderedNamedGridRowLines() { return OrderedNamedGridLinesMap(); }

    static Length initialGridColumnGap() { return Length(Fixed); }
    static Length initialGridRowGap() { return Length(Fixed); }

    // 'auto' is the default.
    static GridPosition initialGridItemColumnStart() { return GridPosition(); }
    static GridPosition initialGridItemColumnEnd() { return GridPosition(); }
    static GridPosition initialGridItemRowStart() { return GridPosition(); }
    static GridPosition initialGridItemRowEnd() { return GridPosition(); }
#endif // ENABLE(CSS_GRID_LAYOUT)

    static unsigned initialTabSize() { return 8; }

    static const AtomicString& initialLineGrid() { return nullAtom; }
    static LineSnap initialLineSnap() { return LineSnapNone; }
    static LineAlign initialLineAlign() { return LineAlignNone; }

    static const AtomicString& initialFlowThread() { return nullAtom; }
    static const AtomicString& initialRegionThread() { return nullAtom; }
    static RegionFragment initialRegionFragment() { return AutoRegionFragment; }

    static IntSize initialInitialLetter() { return IntSize(); }
    static LineClampValue initialLineClamp() { return LineClampValue(); }
    static ETextSecurity initialTextSecurity() { return TSNONE; }

#if PLATFORM(IOS)
    static bool initialTouchCalloutEnabled() { return true; }
#endif

#if ENABLE(TOUCH_EVENTS)
    static Color initialTapHighlightColor();
#endif

#if ENABLE(ACCELERATED_OVERFLOW_SCROLLING)
    static bool initialUseTouchOverflowScrolling() { return false; }
#endif

#if ENABLE(DASHBOARD_SUPPORT)
    static const Vector<StyleDashboardRegion>& initialDashboardRegions();
    static const Vector<StyleDashboardRegion>& noneDashboardRegions();
#endif

    static const FilterOperations& initialFilter() { static NeverDestroyed<FilterOperations> ops; return ops; }

#if ENABLE(FILTERS_LEVEL_2)
    static const FilterOperations& initialBackdropFilter() { static NeverDestroyed<FilterOperations> ops; return ops; }
#endif

#if ENABLE(CSS_COMPOSITING)
    static BlendMode initialBlendMode() { return BlendModeNormal; }
    static Isolation initialIsolation() { return IsolationAuto; }
#endif

    bool isPlaceholderStyle() const { return m_rareNonInheritedData->isPlaceholderStyle; }
    void setIsPlaceholderStyle() { SET_VAR(m_rareNonInheritedData, isPlaceholderStyle, true); }

    void setVisitedLinkColor(const Color&);
    void setVisitedLinkBackgroundColor(const Color& v) { SET_VAR(m_rareNonInheritedData, visitedLinkBackgroundColor, v); }
    void setVisitedLinkBorderLeftColor(const Color& v) { SET_VAR(m_rareNonInheritedData, visitedLinkBorderLeftColor, v); }
    void setVisitedLinkBorderRightColor(const Color& v) { SET_VAR(m_rareNonInheritedData, visitedLinkBorderRightColor, v); }
    void setVisitedLinkBorderBottomColor(const Color& v) { SET_VAR(m_rareNonInheritedData, visitedLinkBorderBottomColor, v); }
    void setVisitedLinkBorderTopColor(const Color& v) { SET_VAR(m_rareNonInheritedData, visitedLinkBorderTopColor, v); }
    void setVisitedLinkOutlineColor(const Color& v) { SET_VAR(m_rareNonInheritedData, visitedLinkOutlineColor, v); }
    void setVisitedLinkColumnRuleColor(const Color& v) { SET_NESTED_VAR(m_rareNonInheritedData, multiCol, visitedLinkColumnRuleColor, v); }
    void setVisitedLinkTextDecorationColor(const Color& v) { SET_VAR(m_rareNonInheritedData, visitedLinkTextDecorationColor, v); }
    void setVisitedLinkTextEmphasisColor(const Color& v) { SET_VAR(m_rareInheritedData, visitedLinkTextEmphasisColor, v); }
    void setVisitedLinkTextFillColor(const Color& v) { SET_VAR(m_rareInheritedData, visitedLinkTextFillColor, v); }
    void setVisitedLinkTextStrokeColor(const Color& v) { SET_VAR(m_rareInheritedData, visitedLinkTextStrokeColor, v); }

    void inheritUnicodeBidiFrom(const RenderStyle* parent) { m_nonInheritedFlags.setUnicodeBidi(parent->m_nonInheritedFlags.unicodeBidi()); }
    void getShadowExtent(const ShadowData*, LayoutUnit& top, LayoutUnit& right, LayoutUnit& bottom, LayoutUnit& left) const;
    void getShadowHorizontalExtent(const ShadowData*, LayoutUnit& left, LayoutUnit& right) const;
    void getShadowVerticalExtent(const ShadowData*, LayoutUnit& top, LayoutUnit& bottom) const;
    void getShadowInlineDirectionExtent(const ShadowData*, LayoutUnit& logicalLeft, LayoutUnit& logicalRight) const;
    void getShadowBlockDirectionExtent(const ShadowData*, LayoutUnit& logicalTop, LayoutUnit& logicalBottom) const;

    static Color invalidColor() { return Color(); }
    const Color& borderLeftColor() const { return m_surroundData->border.left().color(); }
    const Color& borderRightColor() const { return m_surroundData->border.right().color(); }
    const Color& borderTopColor() const { return m_surroundData->border.top().color(); }
    const Color& borderBottomColor() const { return m_surroundData->border.bottom().color(); }
    const Color& backgroundColor() const { return m_backgroundData->color; }
    const Color& color() const;
    const Color& columnRuleColor() const { return m_rareNonInheritedData->multiCol->rule.color(); }
    const Color& outlineColor() const { return m_backgroundData->outline.color(); }
    const Color& textEmphasisColor() const { return m_rareInheritedData->textEmphasisColor; }
    const Color& textFillColor() const { return m_rareInheritedData->textFillColor; }
    const Color& textStrokeColor() const { return m_rareInheritedData->textStrokeColor; }
    const Color& visitedLinkColor() const;
    const Color& visitedLinkBackgroundColor() const { return m_rareNonInheritedData->visitedLinkBackgroundColor; }
    const Color& visitedLinkBorderLeftColor() const { return m_rareNonInheritedData->visitedLinkBorderLeftColor; }
    const Color& visitedLinkBorderRightColor() const { return m_rareNonInheritedData->visitedLinkBorderRightColor; }
    const Color& visitedLinkBorderBottomColor() const { return m_rareNonInheritedData->visitedLinkBorderBottomColor; }
    const Color& visitedLinkBorderTopColor() const { return m_rareNonInheritedData->visitedLinkBorderTopColor; }
    const Color& visitedLinkOutlineColor() const { return m_rareNonInheritedData->visitedLinkOutlineColor; }
    const Color& visitedLinkColumnRuleColor() const { return m_rareNonInheritedData->multiCol->visitedLinkColumnRuleColor; }
    const Color& textDecorationColor() const { return m_rareNonInheritedData->textDecorationColor; }
    const Color& visitedLinkTextDecorationColor() const { return m_rareNonInheritedData->visitedLinkTextDecorationColor; }
    const Color& visitedLinkTextEmphasisColor() const { return m_rareInheritedData->visitedLinkTextEmphasisColor; }
    const Color& visitedLinkTextFillColor() const { return m_rareInheritedData->visitedLinkTextFillColor; }
    const Color& visitedLinkTextStrokeColor() const { return m_rareInheritedData->visitedLinkTextStrokeColor; }

    const Color& stopColor() const { return svgStyle().stopColor(); }
    const Color& floodColor() const { return svgStyle().floodColor(); }
    const Color& lightingColor() const { return svgStyle().lightingColor(); }

private:
    struct NonInheritedFlags {
        NonInheritedFlags();

        bool operator==(const NonInheritedFlags& other) const { return m_flags == other.m_flags; }
        bool operator!=(const NonInheritedFlags& other) const { return m_flags != other.m_flags; }

        void copyNonInheritedFrom(const NonInheritedFlags&);

        EOverflow overflowX() const { return static_cast<EOverflow>(getValue(overflowMask, overflowXOffset)); }
        void setOverflowX(EOverflow overflowX) { updateValue(overflowX, overflowMask, overflowXOffset); }

        EOverflow overflowY() const { return static_cast<EOverflow>(getValue(overflowMask, overflowYOffset)); }
        void setOverflowY(EOverflow overflowY) { updateValue(overflowY, overflowMask, overflowYOffset); }

        EClear clear() const { return static_cast<EClear>(getValue(clearMask, clearOffset)); }
        void setClear(EClear clear) { updateValue(clear, clearMask, clearOffset); }

        EDisplay effectiveDisplay() const { return static_cast<EDisplay>(getValue(displayMask, effectiveDisplayOffset)); }
        void setEffectiveDisplay(EDisplay effectiveDisplay) { updateValue(effectiveDisplay, displayMask, effectiveDisplayOffset); }

        EPosition position() const { return static_cast<EPosition>(getValue(positionMask, positionOffset)); }
        void setPosition(EPosition position) { updateValue(position, positionMask, positionOffset); }

        EDisplay originalDisplay() const { return static_cast<EDisplay>(getValue(displayMask, originalDisplayOffset)); }
        void setOriginalDisplay(EDisplay originalDisplay) { updateValue(originalDisplay, displayMask, originalDisplayOffset); }

        EUnicodeBidi unicodeBidi() const { return static_cast<EUnicodeBidi>(getValue(unicodeBidiMask, unicodeBidiOffset)); }
        void setUnicodeBidi(EUnicodeBidi unicodeBidi) { updateValue(unicodeBidi, unicodeBidiMask, unicodeBidiOffset); }

        bool hasViewportUnits() const { return getBoolean(hasViewportUnitsOffset); }
        void setHasViewportUnits(bool value) { updateBoolean(value, hasViewportUnitsOffset); }

        EVerticalAlign verticalAlign() const { return static_cast<EVerticalAlign>(getValue(verticalAlignMask, verticalAlignOffset)); }
        void setVerticalAlign(EVerticalAlign verticalAlign) { updateValue(verticalAlign, verticalAlignMask, verticalAlignOffset); }

        bool hasExplicitlyInheritedProperties() const { return getBoolean(explicitInheritanceOffset); }
        void setHasExplicitlyInheritedProperties(bool value) { updateBoolean(value, explicitInheritanceOffset); }

        bool isFloating() const { return floating() != NoFloat; }
        EFloat floating() const { return static_cast<EFloat>(getValue(floatingMask, floatingOffset)); }
        void setFloating(EFloat floating) { updateValue(floating, floatingMask, floatingOffset); }

        bool hasAnyPublicPseudoStyles() const { return PUBLIC_PSEUDOID_MASK & getValue(pseudoBitsMask, pseudoBitsOffset); }
        bool hasPseudoStyle(PseudoId) const;
        void setHasPseudoStyle(PseudoId);
        void setHasPseudoStyles(PseudoIdSet);

        ETableLayout tableLayout() const { return static_cast<ETableLayout>(getValue(tableLayoutBitMask, tableLayoutOffset)); }
        void setTableLayout(ETableLayout tableLayout) { updateValue(tableLayout, tableLayoutBitMask, tableLayoutOffset); }

        PseudoId styleType() const { return static_cast<PseudoId>(getValue(styleTypeMask, styleTypeOffset)); }
        void setStyleType(PseudoId styleType) { updateValue(styleType, styleTypeMask, styleTypeOffset); }

        bool isUnique() const { return getBoolean(isUniqueOffset); }
        void setIsUnique() { updateBoolean(true, isUniqueOffset); }

        bool emptyState() const { return getBoolean(emptyStateOffset);  }
        void setEmptyState(bool value) { updateBoolean(value, emptyStateOffset); }

        bool firstChildState() const { return getBoolean(firstChildStateOffset);  }
        void setFirstChildState(bool value) { updateBoolean(value, firstChildStateOffset); }

        bool lastChildState() const { return getBoolean(lastChildStateOffset);  }
        void setLastChildState(bool value) { updateBoolean(value, lastChildStateOffset); }

        bool affectedByHover() const { return getBoolean(affectedByHoverOffset); }
        void setAffectedByHover(bool value) { updateBoolean(value, affectedByHoverOffset); }

        bool affectedByActive() const { return getBoolean(affectedByActiveOffset); }
        void setAffectedByActive(bool value) { updateBoolean(value, affectedByActiveOffset); }

        bool affectedByDrag() const { return getBoolean(affectedByDragOffset); }
        void setAffectedByDrag(bool value) { updateBoolean(value, affectedByDragOffset); }

        bool isLink() const { return getBoolean(isLinkOffset); }
        void setIsLink(bool value) { updateBoolean(value, isLinkOffset); }

        bool hasExplicitlySetDirection() const { return getBoolean(hasExplicitlySetDirectionOffset); }
        void setHasExplicitlySetDirection(bool value) { updateBoolean(value, hasExplicitlySetDirectionOffset); }

        bool hasExplicitlySetWritingMode() const { return getBoolean(hasExplicitlySetWritingModeOffset); }
        void setHasExplicitlySetWritingMode(bool value) { updateBoolean(value, hasExplicitlySetWritingModeOffset); }

        static ptrdiff_t flagsMemoryOffset() { return OBJECT_OFFSETOF(NonInheritedFlags, m_flags); }
        static uint64_t flagIsaffectedByActive() { return oneBitMask << affectedByActiveOffset; }
        static uint64_t flagIsaffectedByHover() { return oneBitMask << affectedByHoverOffset; }
        static uint64_t flagPseudoStyle(PseudoId pseudo) { return oneBitMask << (pseudoBitsOffset - 1 + pseudo); }
        static uint64_t setFirstChildStateFlags() { return flagFirstChildState() | flagIsUnique(); }
        static uint64_t setLastChildStateFlags() { return flagLastChildState() | flagIsUnique(); }

    private:
        void updateBoolean(bool, uint64_t offset);
        bool getBoolean(uint64_t offset) const;
        void updateValue(uint8_t newValue, uint64_t positionIndependentMask, uint64_t offset);
        unsigned getValue(uint64_t positionIndependentMask, uint64_t offset) const;

        static uint64_t flagIsUnique() { return oneBitMask << isUniqueOffset; }
        static uint64_t flagFirstChildState() { return oneBitMask << firstChildStateOffset; }
        static uint64_t flagLastChildState() { return oneBitMask << lastChildStateOffset; }

        // To type the bit mask properly on 64bits.
        static const uint64_t oneBitMask = 0x1;

        // Byte 1.
        static const unsigned overflowBitCount = 3;
        static const uint64_t overflowMask = (oneBitMask << overflowBitCount) - 1;
        static const unsigned overflowXOffset = 0;
        static const unsigned overflowYOffset = overflowXOffset + overflowBitCount;
        static const unsigned clearBitCount = 2;
        static const uint64_t clearMask = (oneBitMask << clearBitCount) - 1;
        static const unsigned clearOffset = overflowYOffset + overflowBitCount;

        // Byte 2.
        static const unsigned displayBitCount = 5;
        static const uint64_t displayMask = (oneBitMask << displayBitCount) - 1;
        static const unsigned effectiveDisplayOffset = clearOffset + clearBitCount;
        static const unsigned positionBitCount = 3;
        static const uint64_t positionMask = (oneBitMask << positionBitCount) - 1;
        static const unsigned positionOffset = effectiveDisplayOffset + displayBitCount;

        // Byte 3.
        static const unsigned originalDisplayOffset = positionOffset + positionBitCount;
        static const unsigned unicodeBidiBitCount = 3;
        static const uint64_t unicodeBidiMask = (oneBitMask << unicodeBidiBitCount) - 1;
        static const unsigned unicodeBidiOffset = originalDisplayOffset + displayBitCount;

        // Byte 4.
        static const unsigned floatingBitCount = 2;
        static const uint64_t floatingMask = (oneBitMask << floatingBitCount) - 1;
        static const unsigned floatingOffset = unicodeBidiOffset + unicodeBidiBitCount;
        static const unsigned hasExplicitlySetDirectionBitcount = 1;
        static const unsigned hasExplicitlySetDirectionOffset = floatingOffset + floatingBitCount;
        static const unsigned hasExplicitlySetWritingModeBitcount = 1;
        static const unsigned hasExplicitlySetWritingModeOffset = hasExplicitlySetDirectionOffset + hasExplicitlySetDirectionBitcount;

        // Byte 5.
        static const unsigned explicitInheritanceBitCount = 1;
        static const unsigned explicitInheritanceOffset = hasExplicitlySetWritingModeOffset + hasExplicitlySetWritingModeBitcount;
        static const unsigned tableLayoutBitCount = 1;
        static const uint64_t tableLayoutBitMask = oneBitMask;
        static const unsigned tableLayoutOffset = explicitInheritanceOffset + explicitInheritanceBitCount;
        static const unsigned verticalAlignBitCount = 4;
        static const unsigned verticalAlignPadding = 2;
        static const unsigned verticalAlignAndPaddingBitCount = verticalAlignBitCount + verticalAlignPadding;
        static const uint64_t verticalAlignMask = (oneBitMask << verticalAlignBitCount) - 1;
        static const unsigned verticalAlignOffset = tableLayoutOffset + tableLayoutBitCount;

        // Byte 6.
        static const unsigned pseudoBitsBitCount = 7;
        static const uint64_t pseudoBitsMask = (oneBitMask << pseudoBitsBitCount) - 1;
        static const unsigned pseudoBitsOffset = verticalAlignOffset + verticalAlignBitCount;

        static const unsigned hasViewportUnitsBitCount = 1;
        static const uint64_t hasViewportUnitsBitMask = (oneBitMask << hasViewportUnitsBitCount) - 1;
        static const unsigned hasViewportUnitsOffset = pseudoBitsOffset + pseudoBitsBitCount;

        // Byte 7.
        static const unsigned styleTypeBitCount = 6;
        static const unsigned styleTypePadding = 2;
        static const unsigned styleTypeAndPaddingBitCount = styleTypeBitCount + styleTypePadding;
        static const uint64_t styleTypeMask = (oneBitMask << styleTypeAndPaddingBitCount) - 1;
        static const unsigned styleTypeOffset = hasViewportUnitsBitCount + hasViewportUnitsOffset;

        // Byte 8.
        static const unsigned isUniqueOffset = styleTypeOffset + styleTypeAndPaddingBitCount;
        static const unsigned emptyStateOffset = isUniqueOffset + 1;
        static const unsigned firstChildStateOffset = emptyStateOffset + 1;
        static const unsigned lastChildStateOffset = firstChildStateOffset + 1;
        static const unsigned affectedByHoverOffset = lastChildStateOffset + 1;
        static const unsigned affectedByActiveOffset = affectedByHoverOffset + 1;
        static const unsigned affectedByDragOffset = affectedByActiveOffset + 1;
        static const unsigned isLinkOffset = affectedByDragOffset + 1;

        // 60 bits are assigned. There are 4 bits available currently used as padding to improve code generation.
        // If you add more style bits here, you will also need to update RenderStyle::copyNonInheritedFrom().
        uint64_t m_flags { 0 };
    };

    struct InheritedFlags {
        bool operator==(const InheritedFlags&) const;
        bool operator!=(const InheritedFlags& other) const { return !(*this == other); }

        unsigned emptyCells : 1; // EEmptyCell
        unsigned captionSide : 2; // ECaptionSide
        unsigned listStyleType : 7; // EListStyleType
        unsigned listStylePosition : 1; // EListStylePosition
        unsigned visibility : 2; // EVisibility
        unsigned textAlign : 4; // ETextAlign
        unsigned textTransform : 2; // ETextTransform
        unsigned textDecorations : TextDecorationBits;
        unsigned cursor : 6; // ECursor
#if ENABLE(CURSOR_VISIBILITY)
        unsigned cursorVisibility : 1; // CursorVisibility
#endif
        unsigned direction : 1; // TextDirection
        unsigned whiteSpace : 3; // EWhiteSpace
        // 32 bits
        unsigned borderCollapse : 1; // EBorderCollapse
        unsigned boxDirection : 1; // EBoxDirection (CSS3 box_direction property, flexible box layout module)

        // non CSS2 inherited
        unsigned rtlOrdering : 1; // Order
        unsigned printColorAdjust : PrintColorAdjustBits;
        unsigned pointerEvents : 4; // EPointerEvents
        unsigned insideLink : 2; // EInsideLink
        unsigned insideDefaultButton : 1;
        // 44 bits

        // CSS Text Layout Module Level 3: Vertical writing support
        unsigned writingMode : 2; // WritingMode
        // 46 bits
    };

    // This constructor is used to implement the replace operation.
    RenderStyle(RenderStyle&, RenderStyle&&);

    EDisplay originalDisplay() const { return m_nonInheritedFlags.originalDisplay(); }

    bool hasAutoLeftAndRight() const { return left().isAuto() && right().isAuto(); }
    bool hasAutoTopAndBottom() const { return top().isAuto() && bottom().isAuto(); }

    void setContent(std::unique_ptr<ContentData>, bool add);

    LayoutBoxExtent getShadowInsetExtent(const ShadowData*) const;

    static bool isDisplayReplacedType(EDisplay);
    static bool isDisplayInlineType(EDisplay);
    static bool isDisplayFlexibleBox(EDisplay);
    static bool isDisplayGridBox(EDisplay);
    static bool isDisplayFlexibleOrGridBox(EDisplay);

    Color colorIncludingFallback(int colorProperty, bool visitedLink) const;

    bool changeAffectsVisualOverflow(const RenderStyle&) const;
    bool changeRequiresLayout(const RenderStyle&, unsigned& changedContextSensitiveProperties) const;
    bool changeRequiresPositionedLayoutOnly(const RenderStyle&, unsigned& changedContextSensitiveProperties) const;
    bool changeRequiresLayerRepaint(const RenderStyle&, unsigned& changedContextSensitiveProperties) const;
    bool changeRequiresRepaint(const RenderStyle&, unsigned& changedContextSensitiveProperties) const;
    bool changeRequiresRepaintIfTextOrBorderOrOutline(const RenderStyle&, unsigned& changedContextSensitiveProperties) const;
    bool changeRequiresRecompositeLayer(const RenderStyle&, unsigned& changedContextSensitiveProperties) const;

    // non-inherited attributes
    DataRef<StyleBoxData> m_boxData;
    DataRef<StyleVisualData> m_visualData;
    DataRef<StyleBackgroundData> m_backgroundData;
    DataRef<StyleSurroundData> m_surroundData;
    DataRef<StyleRareNonInheritedData> m_rareNonInheritedData;
    NonInheritedFlags m_nonInheritedFlags;

    // inherited attributes
    DataRef<StyleRareInheritedData> m_rareInheritedData;
    DataRef<StyleInheritedData> m_inheritedData;
    InheritedFlags m_inheritedFlags;

    // list of associated pseudo styles
    std::unique_ptr<PseudoStyleCache> m_cachedPseudoStyles;

    DataRef<SVGRenderStyle> m_svgStyle;

#if !ASSERT_DISABLED || ENABLE(SECURITY_ASSERTIONS)
    bool m_deletionHasBegun { false };
#endif
};

int adjustForAbsoluteZoom(int, const RenderStyle&);
float adjustFloatForAbsoluteZoom(float, const RenderStyle&);
LayoutUnit adjustLayoutUnitForAbsoluteZoom(LayoutUnit, const RenderStyle&);

EBorderStyle collapsedBorderStyle(EBorderStyle);

bool pseudoElementRendererIsNeeded(const RenderStyle*);

inline RenderStyle::NonInheritedFlags::NonInheritedFlags()
{
    // All these initial values need to be zero so that zero-initializing m_flags is correct.
    ASSERT(!initialOverflowX());
    ASSERT(!initialOverflowY());
    ASSERT(!initialClear());
    ASSERT(!initialDisplay());
    ASSERT(!initialUnicodeBidi());
    ASSERT(!initialPosition());
    ASSERT(!initialVerticalAlign());
    ASSERT(!initialFloating());
    ASSERT(!initialTableLayout());
}

inline void RenderStyle::NonInheritedFlags::copyNonInheritedFrom(const NonInheritedFlags& other)
{
    // Only a subset is copied because NonInheritedFlags contains a bunch of stuff other than real style data.
    uint64_t nonInheritedMask = overflowMask << overflowXOffset
        | overflowMask << overflowYOffset
        | clearMask << clearOffset
        | displayMask << effectiveDisplayOffset
        | positionMask << positionOffset
        | displayMask << originalDisplayOffset
        | unicodeBidiMask << unicodeBidiOffset
        | verticalAlignMask << verticalAlignOffset
        | floatingMask << floatingOffset
        | oneBitMask << explicitInheritanceOffset
        | tableLayoutBitMask << tableLayoutOffset
        | hasViewportUnitsBitMask << hasViewportUnitsOffset;

    m_flags = (m_flags & ~nonInheritedMask) | (other.m_flags & nonInheritedMask);
}

inline bool RenderStyle::NonInheritedFlags::hasPseudoStyle(PseudoId pseudo) const
{
    ASSERT(pseudo > NOPSEUDO);
    ASSERT(pseudo < FIRST_INTERNAL_PSEUDOID);
    return (oneBitMask << (pseudoBitsOffset - 1 + pseudo)) & m_flags;
}

inline void RenderStyle::NonInheritedFlags::setHasPseudoStyle(PseudoId pseudo)
{
    ASSERT(pseudo > NOPSEUDO);
    ASSERT(pseudo < FIRST_INTERNAL_PSEUDOID);
    m_flags |= oneBitMask << (pseudoBitsOffset - 1 + pseudo);
}

inline void RenderStyle::NonInheritedFlags::setHasPseudoStyles(PseudoIdSet pseudoIdSet)
{
    ASSERT(pseudoIdSet);
    uint64_t rawPseudoIdSet = pseudoIdSet.data();
    ASSERT((rawPseudoIdSet & PUBLIC_PSEUDOID_MASK) == rawPseudoIdSet);
    static_assert(pseudoBitsOffset >= 1, "(pseudoBitsOffset - 1) should be valid.");
    m_flags |= (static_cast<uint64_t>(rawPseudoIdSet) << (pseudoBitsOffset - 1));
}

inline void RenderStyle::NonInheritedFlags::updateBoolean(bool isSet, uint64_t offset)
{
    if (isSet)
        m_flags |= (oneBitMask << offset);
    else
        m_flags &= ~(oneBitMask << offset);
}

inline bool RenderStyle::NonInheritedFlags::getBoolean(uint64_t offset) const
{
    return m_flags & (oneBitMask << offset);
}

inline void RenderStyle::NonInheritedFlags::updateValue(uint8_t newValue, uint64_t positionIndependentMask, uint64_t offset)
{
    ASSERT(!(newValue & ~positionIndependentMask));
    uint64_t positionDependentMask = positionIndependentMask << offset;
    m_flags = (m_flags & ~positionDependentMask) | (static_cast<uint64_t>(newValue) << offset);
}

inline bool RenderStyle::InheritedFlags::operator==(const InheritedFlags& other) const
{
    return emptyCells == other.emptyCells
        && captionSide == other.captionSide
        && listStyleType == other.listStyleType
        && listStylePosition == other.listStylePosition
        && visibility == other.visibility
        && textAlign == other.textAlign
        && textTransform == other.textTransform
        && textDecorations == other.textDecorations
        && cursor == other.cursor
#if ENABLE(CURSOR_VISIBILITY)
        && cursorVisibility == other.cursorVisibility
#endif
        && direction == other.direction
        && whiteSpace == other.whiteSpace
        && borderCollapse == other.borderCollapse
        && boxDirection == other.boxDirection
        && rtlOrdering == other.rtlOrdering
        && printColorAdjust == other.printColorAdjust
        && pointerEvents == other.pointerEvents
        && insideLink == other.insideLink
        && insideDefaultButton == other.insideDefaultButton
        && writingMode == other.writingMode;
}

inline unsigned RenderStyle::NonInheritedFlags::getValue(uint64_t positionIndependentMask, uint64_t offset) const
{
    return static_cast<unsigned>((m_flags >> offset) & positionIndependentMask);
}

inline int adjustForAbsoluteZoom(int value, const RenderStyle& style)
{
    double zoomFactor = style.effectiveZoom();
    if (zoomFactor == 1)
        return value;
    // Needed because computeLengthInt truncates (rather than rounds) when scaling up.
    if (zoomFactor > 1) {
        if (value < 0)
            value--;
        else 
            value++;
    }

    return roundForImpreciseConversion<int>(value / zoomFactor);
}

inline float adjustFloatForAbsoluteZoom(float value, const RenderStyle& style)
{
    return value / style.effectiveZoom();
}

inline LayoutUnit adjustLayoutUnitForAbsoluteZoom(LayoutUnit value, const RenderStyle& style)
{
    return value / style.effectiveZoom();
}

inline EBorderStyle collapsedBorderStyle(EBorderStyle style)
{
    if (style == OUTSET)
        return GROOVE;
    if (style == INSET)
        return RIDGE;
    return style;
}

inline bool RenderStyle::hasBackground() const
{
    return visitedDependentColor(CSSPropertyBackgroundColor).isVisible() ||  hasBackgroundImage();
}

inline bool RenderStyle::autoWrap(EWhiteSpace whiteSpace)
{
    // Nowrap and pre don't automatically wrap.
    return whiteSpace != NOWRAP && whiteSpace != PRE;
}

inline bool RenderStyle::preserveNewline(EWhiteSpace whiteSpace)
{
    // Normal and nowrap do not preserve newlines.
    return whiteSpace != NORMAL && whiteSpace != NOWRAP;
}

inline bool RenderStyle::collapseWhiteSpace(EWhiteSpace ws)
{
    // Pre and prewrap do not collapse whitespace.
    return ws != PRE && ws != PRE_WRAP;
}

inline bool RenderStyle::isCollapsibleWhiteSpace(UChar character) const
{
    switch (character) {
    case ' ':
    case '\t':
        return collapseWhiteSpace();
    case '\n':
        return !preserveNewline();
    default:
        return false;
    }
}

inline bool RenderStyle::breakOnlyAfterWhiteSpace() const
{
    return whiteSpace() == PRE_WRAP || lineBreak() == LineBreakAfterWhiteSpace;
}

inline bool RenderStyle::breakWords() const
{
    return wordBreak() == BreakWordBreak || overflowWrap() == BreakOverflowWrap;
}

inline bool RenderStyle::hasInlineColumnAxis() const
{
    auto axis = columnAxis();
    return axis == AutoColumnAxis || isHorizontalWritingMode() == (axis == HorizontalColumnAxis);
}

inline ImageOrientationEnum RenderStyle::imageOrientation() const
{
#if ENABLE(CSS_IMAGE_ORIENTATION)
    return static_cast<ImageOrientationEnum>(m_rareInheritedData->imageOrientation);
#else
    return DefaultImageOrientation;
#endif
}

inline void RenderStyle::setLogicalWidth(Length&& logicalWidth)
{
    if (isHorizontalWritingMode())
        setWidth(WTFMove(logicalWidth));
    else
        setHeight(WTFMove(logicalWidth));
}

inline void RenderStyle::setLogicalHeight(Length&& logicalHeight)
{
    if (isHorizontalWritingMode())
        setHeight(WTFMove(logicalHeight));
    else
        setWidth(WTFMove(logicalHeight));
}

inline void RenderStyle::setBorderRadius(LengthSize&& size)
{
    auto topLeft = size;
    setBorderTopLeftRadius(WTFMove(topLeft));
    auto topRight = size;
    setBorderTopRightRadius(WTFMove(topRight));
    auto bottomLeft = size;
    setBorderBottomLeftRadius(WTFMove(bottomLeft));
    setBorderBottomRightRadius(WTFMove(size));
}

inline void RenderStyle::setBorderRadius(const IntSize& size)
{
    setBorderRadius(LengthSize { { size.width(), Fixed }, { size.height(), Fixed } });
}

inline bool RenderStyle::setZoom(float zoomLevel)
{
    setEffectiveZoom(effectiveZoom() * zoomLevel);
    if (compareEqual(m_visualData->zoom, zoomLevel))
        return false;
    m_visualData.access().zoom = zoomLevel;
    return true;
}

inline bool RenderStyle::setEffectiveZoom(float zoomLevel)
{
    if (compareEqual(m_rareInheritedData->effectiveZoom, zoomLevel))
        return false;
    m_rareInheritedData.access().effectiveZoom = zoomLevel;
    return true;
}

inline bool RenderStyle::setTextOrientation(TextOrientation textOrientation)
{
    if (compareEqual(m_rareInheritedData->textOrientation, static_cast<unsigned>(textOrientation)))
        return false;
    m_rareInheritedData.access().textOrientation = static_cast<unsigned>(textOrientation);
    return true;
}

inline void RenderStyle::adjustBackgroundLayers()
{
    if (backgroundLayers().next()) {
        ensureBackgroundLayers().cullEmptyLayers();
        ensureBackgroundLayers().fillUnsetProperties();
    }
}

inline void RenderStyle::adjustMaskLayers()
{
    if (maskLayers().next()) {
        ensureMaskLayers().cullEmptyLayers();
        ensureMaskLayers().fillUnsetProperties();
    }
}

inline void RenderStyle::clearAnimations()
{
    m_rareNonInheritedData.access().animations = nullptr;
}

inline void RenderStyle::clearTransitions()
{
    m_rareNonInheritedData.access().transitions = nullptr;
}

inline void RenderStyle::setShapeOutside(RefPtr<ShapeValue>&& value)
{
    if (m_rareNonInheritedData->shapeOutside == value)
        return;
    m_rareNonInheritedData.access().shapeOutside = WTFMove(value);
}

inline void RenderStyle::setShapeImageThreshold(float shapeImageThreshold)
{
    float clampedShapeImageThreshold = clampTo<float>(shapeImageThreshold, 0, 1);
    SET_VAR(m_rareNonInheritedData, shapeImageThreshold, clampedShapeImageThreshold);
}

inline void RenderStyle::setClipPath(RefPtr<ClipPathOperation>&& operation)
{
    if (m_rareNonInheritedData->clipPath != operation)
        m_rareNonInheritedData.access().clipPath = WTFMove(operation);
}

inline bool RenderStyle::willChangeCreatesStackingContext() const
{
    return willChange() && willChange()->canCreateStackingContext();
}

inline bool RenderStyle::isDisplayRegionType() const
{
    return display() == BLOCK || display() == INLINE_BLOCK
        || display() == TABLE_CELL || display() == TABLE_CAPTION
        || display() == LIST_ITEM;
}

inline bool RenderStyle::setWritingMode(WritingMode v)
{
    if (v == writingMode())
        return false;
    m_inheritedFlags.writingMode = v;
    return true;
}

inline void RenderStyle::getShadowInlineDirectionExtent(const ShadowData* shadow, LayoutUnit& logicalLeft, LayoutUnit& logicalRight) const
{
    return isHorizontalWritingMode() ? getShadowHorizontalExtent(shadow, logicalLeft, logicalRight) : getShadowVerticalExtent(shadow, logicalLeft, logicalRight);
}

inline void RenderStyle::getShadowBlockDirectionExtent(const ShadowData* shadow, LayoutUnit& logicalTop, LayoutUnit& logicalBottom) const
{
    return isHorizontalWritingMode() ? getShadowVerticalExtent(shadow, logicalTop, logicalBottom) : getShadowHorizontalExtent(shadow, logicalTop, logicalBottom);
}

inline bool RenderStyle::isDisplayReplacedType(EDisplay display)
{
    return display == INLINE_BLOCK || display == INLINE_BOX || display == INLINE_FLEX
#if ENABLE(CSS_GRID_LAYOUT)
        || display == INLINE_GRID
#endif
        || display == INLINE_TABLE;
}

inline bool RenderStyle::isDisplayInlineType(EDisplay display)
{
    return display == INLINE || isDisplayReplacedType(display);
}

inline bool RenderStyle::isDisplayFlexibleBox(EDisplay display)
{
    return display == FLEX || display == INLINE_FLEX;
}

inline bool RenderStyle::isDisplayGridBox(EDisplay display)
{
#if ENABLE(CSS_GRID_LAYOUT)
    return display == GRID || display == INLINE_GRID;
#else
    UNUSED_PARAM(display);
    return false;
#endif
}

inline bool RenderStyle::isDisplayFlexibleOrGridBox(EDisplay display)
{
    return isDisplayFlexibleBox(display) || isDisplayGridBox(display);
}

inline bool RenderStyle::hasAnyPublicPseudoStyles() const
{
    return m_nonInheritedFlags.hasAnyPublicPseudoStyles();
}

inline bool RenderStyle::hasPseudoStyle(PseudoId pseudo) const
{
    return m_nonInheritedFlags.hasPseudoStyle(pseudo);
}

inline void RenderStyle::setHasPseudoStyle(PseudoId pseudo)
{
    m_nonInheritedFlags.setHasPseudoStyle(pseudo);
}

inline void RenderStyle::setHasPseudoStyles(PseudoIdSet pseudoIdSet)
{
    m_nonInheritedFlags.setHasPseudoStyles(pseudoIdSet);
}

inline void RenderStyle::setBoxReflect(RefPtr<StyleReflection>&& reflect)
{
    SET_VAR(m_rareNonInheritedData, boxReflect, WTFMove(reflect));
}

inline bool pseudoElementRendererIsNeeded(const RenderStyle* style)
{
    return style && style->display() != NONE && (style->contentData() || style->hasFlowFrom());
}

} // namespace WebCore
