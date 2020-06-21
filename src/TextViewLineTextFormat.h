//
// Created by kca on 21/6/2020.
//

#ifndef SDV_TEXTVIEWLINETEXTFORMAT_H
#define SDV_TEXTVIEWLINETEXTFORMAT_H

#include <memory>
#include "TextViewTextFormat.h"

namespace SDV {

struct TextViewLineTextFormat
{
    int charIndex;
    int length;
    std::shared_ptr<TextViewTextFormat> format;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWLINETEXTFORMAT_H
