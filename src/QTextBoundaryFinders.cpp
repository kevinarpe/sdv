//
// Created by kca on 19/4/2020.
//

#include "QTextBoundaryFinders.h"

namespace SDV {

/**
 * @param string
 *        empty is allowed and will return zero
 */
// public static
int
QTextBoundaryFinders::
countBoundaries(QTextBoundaryFinder::BoundaryType type, const QString& string)
{
    QTextBoundaryFinder f{type, string};
    int count = 0;
    while (-1 != f.toNextBoundary()) {
        ++count;
    }
    return count;
}

}  // namespace SDV
