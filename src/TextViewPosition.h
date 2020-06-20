//
// Created by kca on 14/6/2020.
//

#ifndef SDV_TEXTVIEWPOSITION_H
#define SDV_TEXTVIEWPOSITION_H

#include <compare>
#include "GraphemeFinder.h"

namespace SDV {

struct TextViewPosition
{
    /**
     * Zero-based index of line.  This is not a logical line index that is affected by preceding hidden lines.
     *
     * For example, if the value is two, this represents the third line of text.  However, the preceding two lines may
     * be hidden.  Thus, the line may appear as the first logical line at index zero.
     */
    int lineIndex;
    /**
     * Within line of text at lineIndex, this is the zero-based offset of the first grapheme QChar.
     *
     * For example: In the text "😋r", the letter "r" would have QChar index 2,
     * whereas the emoji character would use QChar indices 0 and 1.
     */
    int charIndex;
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
     */
    QString grapheme;

    bool isValid() const
    {
        const bool x = (lineIndex >= 0 && charIndex >= 0 && graphemeIndex >= 0 && grapheme.length() > 0);
        return x;
    }

    void invalidate() { *this = invalid(); }

    bool isEqual(const TextViewPosition& rhs) const
    {
        const bool x = lineIndex == rhs.lineIndex
                       && charIndex == rhs.charIndex
                       && graphemeIndex == rhs.graphemeIndex
                       && grapheme == rhs.grapheme;
        return x;
    }

    bool isLessThan(const TextViewPosition& rhs) const
    {
        if (lineIndex < rhs.lineIndex) {
            return true;
        }
        else if (lineIndex == rhs.lineIndex && charIndex < rhs.charIndex) {
            return true;
        }
        else {
            return false;
        }
    }

    static TextViewPosition invalid() {
        return TextViewPosition{.lineIndex = -1, .charIndex = -1, .graphemeIndex = -1, .grapheme = QString{}};
    }
};

}  // namespace SDV

#endif //SDV_TEXTVIEWPOSITION_H
