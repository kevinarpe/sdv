//
// Created by kca on 2/5/2020.
//

#ifndef SDV_TEXTFORMAT_H
#define SDV_TEXTFORMAT_H

#include <QTextCharFormat>

namespace SDV {

struct TextFormat
{
    QTextCharFormat textCharFormat;
    QString richTextPrefix;
    QString richTextSuffix;
};

}  // namespace SDV

#endif //SDV_TEXTFORMAT_H
