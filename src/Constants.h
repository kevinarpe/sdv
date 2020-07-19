//
// Created by kca on 25/4/2020.
//

#ifndef SDV_CONSTANTS_H
#define SDV_CONSTANTS_H

#include <QString>

namespace SDV {

enum class JsonNodeType
{
    Null = 1,
    Bool = 2,
    Number = 4,
    String = 8,
    Key = 16,
    ObjectBegin = 32,
    ObjectEnd = 64,
    ArrayBegin = 128,
    ArrayEnd = 256,
};

// Ref: https://doc.qt.io/qt-5/qflags.html
Q_DECLARE_FLAGS(JsonNodeTypes, JsonNodeType)
Q_DECLARE_OPERATORS_FOR_FLAGS(JsonNodeTypes)

struct IsRichText
{
    static const IsRichText kYes;
    static const IsRichText kNo;

    const Qt::TextFormat textFormat;
private:
    explicit IsRichText(const bool isRichText)
        : textFormat(isRichText ? Qt::TextFormat::RichText : Qt::TextFormat::PlainText)
        {}
};

struct Constants
{
    static const QString kStdinFileName;  // {QLatin1Char{'-'}}
    static const bool kIsMicrosoftWindows;
    static const QString kNewLine;  // {QLatin1Char{'\n'}} or {QLatin1String{"\r\n"}}

    struct Json
    {
        static const QString kObjectBegin;  // {QLatin1Char{'{'}
        static const QString kObjectEnd;    // {QLatin1Char{'}'}
        static const QString kArrayBegin;   // {QLatin1Char{'['}
        static const QString kArrayEnd;     // {QLatin1Char{']'}
    };
};

}  // namespace SDV

#endif //SDV_CONSTANTS_H
