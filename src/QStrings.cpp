//
// Created by kca on 9/7/2020.
//

#include "QStrings.h"

namespace SDV {

// public static
int
QStrings::
utf8ByteCount(const QString& s, const int offset, const int length)
{
    const int codePointCount = s.size();
    assert(offset >= 0 && (offset + length) <= codePointCount);

    const QChar* data = s.constData();
    int byteCount = 0;

    for (int i = 0; i < codePointCount; ++i, ++data)
    {
        ++byteCount;
        if (data->unicode() > 0xff) {
            ++byteCount;
        }
    }
    return byteCount;
}

}  // namespace SDV
