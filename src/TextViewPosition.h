//
// Created by kca on 21/6/2020.
//

#ifndef SDV_TEXTVIEWPOSITION_H
#define SDV_TEXTVIEWPOSITION_H

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

    bool isValid() const
    {
        const bool x = (lineIndex >= 0 && charIndex >= 0);
        return x;
    }

    void invalidate() { *this = invalid(); }

    bool isEqual(const TextViewPosition& rhs) const
    {
        const bool x = (lineIndex == rhs.lineIndex && charIndex == rhs.charIndex);
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

    static TextViewPosition invalid() { return TextViewPosition{.lineIndex = -1, .charIndex = -1}; }
};

}  // namespace SDV

#endif //SDV_TEXTVIEWPOSITION_H
