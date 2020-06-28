//
// Created by kca on 14/6/2020.
//

#ifndef SDV_TEXTVIEWGRAPHEMEPOSITION_H
#define SDV_TEXTVIEWGRAPHEMEPOSITION_H

#include <QString>
#include "TextViewPosition.h"

namespace SDV {

struct TextViewGraphemePosition
{
    /** Grapheme when cursor is at end-of-line */
    static const QString kEndOfLineGrapheme;  // {kEndOfLineChar}

    TextViewPosition pos;
    /**
     * Zero based logical column index for a visual text editor.
     *
     * For example: In the text "😋r", the letter 'r' would have grapheme index 1,
     * whereas the emoji character would use grapheme index 0.
     */
    int graphemeIndex;

    /**
     * Always length 1 or 2
     *
     * For example: The grapheme "r" has length 1, but the grapheme "😋" has length 2.
     *
     * @see #isEndOfLine
     */
    QString grapheme;

    /**
     * If true, then {@link #grapheme} is always {@link #kEndOfLineGrapheme}.
     */
    bool isEndOfLine;

    bool isValid() const
    {
        const bool x = (pos.isValid() && graphemeIndex >= 0 && grapheme.length() > 0);
        return x;
    }

    static TextViewGraphemePosition invalid()
    {
        return TextViewGraphemePosition{.pos = TextViewPosition::invalid(), .graphemeIndex = -1, .grapheme = QString{}};
    }
};

}  // namespace SDV

#endif //SDV_TEXTVIEWGRAPHEMEPOSITION_H
