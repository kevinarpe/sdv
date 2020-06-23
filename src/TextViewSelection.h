//
// Created by kca on 20/6/2020.
//

#ifndef SDV_TEXTVIEWSELECTION_H
#define SDV_TEXTVIEWSELECTION_H

#include "TextViewPosition.h"

namespace SDV {

struct TextViewSelection
{
    TextViewPosition begin;
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
