//
// Created by kca on 4/7/2020.
//

#ifndef SDV_TEXTVIEWSELECTIONRANGE_H
#define SDV_TEXTVIEWSELECTIONRANGE_H

#include "TextViewSelection.h"

namespace SDV {

struct TextViewSelectionRange
{
    const TextViewSelection selection;

    /** Inclusive and normalised for backward selection -- always less or equal to lastLineIndex. */
    const int firstLineIndex;

    /** Inclusive and normalised for backward selection -- always greater or equal to lastLineIndex. */
    const int lastLineIndex;

    explicit TextViewSelectionRange(const TextViewSelection& selection);

    bool isValid() const { return (firstLineIndex >= 0 && lastLineIndex >= 0); }

    bool
    contains(const int lineIndex)
    const
    {
        const bool x = (isValid() && lineIndex >= firstLineIndex && lineIndex <= lastLineIndex);
        return x;
    }
};

}  // namespace SDV

#endif //SDV_TEXTVIEWSELECTIONRANGE_H
