//
// Created by kca on 9/7/2020.
//

#ifndef SDV_QSTRINGS_H
#define SDV_QSTRINGS_H

#include <QString>

namespace SDV {

struct QStrings
{
    static int utf8ByteCount(const QString& s) { return utf8ByteCount(s, 0, s.length()); }

    static int utf8ByteCount(const QString& s, int offset, int length);
};

}  // namespace SDV

#endif //SDV_QSTRINGS_H
