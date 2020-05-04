//
// Created by kca on 19/4/2020.
//

#ifndef SDV_QTEXTBOUNDARYFINDERS_H
#define SDV_QTEXTBOUNDARYFINDERS_H

#include <QTextBoundaryFinder>

namespace SDV {

struct QTextBoundaryFinders
{
    static int
    countBoundaries(QTextBoundaryFinder::BoundaryType type, const QString& string);
};

}  // namespace SDV

#endif //SDV_QTEXTBOUNDARYFINDERS_H
