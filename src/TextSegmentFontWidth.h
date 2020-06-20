//
// Created by kca on 18/6/2020.
//

#ifndef SDV_TEXTSEGMENTFONTWIDTH_H
#define SDV_TEXTSEGMENTFONTWIDTH_H

#include <QtGlobal>

namespace SDV {

struct TextSegmentFontWidth
{
    /**
     * Width (font horizontal advance) of text before cursor.
     *
     * When moving up and down, good text editors use min(target, actual) for column index.
     * Target is only updated for horizontal movement, but not for vertical movement.
     */
    qreal beforeGrapheme;
    /** Width (font horizontal advance) of char under cursor. */
    qreal grapheme;

    bool isValid() const { return beforeGrapheme >= 0 && grapheme > 0; }

    static TextSegmentFontWidth invalid() { return TextSegmentFontWidth{.beforeGrapheme = -1, .grapheme = 0}; }
};

}  // namespace SDV

#endif //SDV_TEXTSEGMENTFONTWIDTH_H
