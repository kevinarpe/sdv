//
// Created by kca on 14/6/2020.
//

#ifndef SDV_TEXTVIEWSELECTION_H
#define SDV_TEXTVIEWSELECTION_H

#include "TextViewPosition.h"

namespace SDV {

struct TextViewSelection
{
    TextViewPosition beginInclusive;
    TextViewPosition endExclusive;

    bool isValid() const
    {
        const bool x = beginInclusive.isValid() && endExclusive.isValid();
        return x;
    }

    bool containsLineIndex(const int lineIndex) const
    {
        const bool x = isValid() && lineIndex >= beginInclusive.lineIndex && lineIndex < endExclusive.lineIndex;
        return x;
    }
};

}  // namespace SDV

#endif //SDV_TEXTVIEWSELECTION_H
