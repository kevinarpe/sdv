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
    const int size = s.size();
    assert(offset >= 0 && (offset + length) <= size);

    const QChar* data = s.constData();
    int byteCount = 0;

    for (int i = offset; i < offset + length; ++i)
    {
        ++byteCount;
        const QChar& ch = data[i];
        if (ch.unicode() > 0xff) {
            ++byteCount;
        }
    }
    return byteCount;
}

}  // namespace SDV
