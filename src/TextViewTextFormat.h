//
// Created by kca on 21/6/2020.
//

#ifndef SDV_TEXTVIEWTEXTFORMAT_H
#define SDV_TEXTVIEWTEXTFORMAT_H

#include <optional>
#include <QFont>
#include <QPen>

namespace SDV {

struct TextViewTextFormat
{
    std::optional<QFont> font;
    std::optional<QPen> foregroundPen;
//    std::optional<QBrush> backgroundBrush;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWTEXTFORMAT_H
