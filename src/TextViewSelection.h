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
        const bool x = begin.isValid() && end.isValid();
        return x;
    }

    void invalidate()
    {
        begin.invalidate();
        end.invalidate();
    }

    static TextViewSelection invalid()
    {
        return TextViewSelection{.begin = TextViewPosition::invalid(), .end = TextViewPosition::invalid()};
    }
};

}  // namespace SDV

#endif //SDV_TEXTVIEWSELECTION_H
