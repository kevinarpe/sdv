//
// Created by kca on 20/6/2020.
//

#ifndef SDV_TEXTVIEWSELECTION_H
#define SDV_TEXTVIEWSELECTION_H

#include "TextViewPosition.h"

namespace SDV {

struct TextViewSelection
{
    /**
     * This value might be (a) greater than, (b) equal to, or (c) less than to {@link #end}.
     * <p>
     * When {@link #isValid()} is {@code true}:
     * <ul>
     * <li>If this value is less than {@code end}, then this value is <b>inclusive</b>.</li>
     * <li>If this value is greater than {@code end}, then this value is <b>exclusive</b>.</li>
     * </ul>
     */
    TextViewPosition begin;
    /**
     * See {@link #begin} to understand inclusive vs exclusive.
     * <p>
     * In short, when {@link #isValid()} is {@code true}, then one is inclusive, and the other is exclusive.
     */
    TextViewPosition end;

    bool isValid() const
    {
        const bool x = begin.isValid() && end.isValid() && false == begin.isEqual(end);
        return x;
    }

    void invalidate()
    {
        begin.invalidate();
        end.invalidate();
    }

    bool containsLine(const int lineIndex) const
    {
        if (false == isValid()) {
            return false;
        }

        if (begin.isLessThan(end))
        {
            const bool x = (lineIndex >= begin.lineIndex && lineIndex <= end.lineIndex);
            return x;
        }
        else {
            const bool x = (lineIndex >= end.lineIndex && lineIndex <= begin.lineIndex);
            return x;
        }
    }

    bool isEqual(const TextViewSelection& rhs) const
    {
        const bool x = (begin.isEqual(rhs.begin) && end.isEqual(rhs.end));
        return x;
    }

    static TextViewSelection invalid()
    {
        return TextViewSelection{.begin = TextViewPosition::invalid(), .end = TextViewPosition::invalid()};
    }
};

}  // namespace SDV

#endif //SDV_TEXTVIEWSELECTION_H
