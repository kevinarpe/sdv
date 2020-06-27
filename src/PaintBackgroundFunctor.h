//
// Created by kca on 25/6/2020.
//

#ifndef SDV_PAINTBACKGROUNDFUNCTOR_H
#define SDV_PAINTBACKGROUNDFUNCTOR_H

class QPainter;
class QRectF;
#include "LineFormat.h"

namespace SDV {

class PaintContext;

struct PaintBackgroundFunctor
{
    virtual ~PaintBackgroundFunctor() = 0;

    /**
     * Call {@code QPainter::fillRect(...)} and/or {@code QPainter::drawRect(...)}
     */
    virtual void draw(QPainter& painter, PaintContext* nullableContext, const QRectF& textBoundingRect) const = 0;
};

using LineFormatBackground = LineFormat<PaintBackgroundFunctor>;

}  // namespace SDV

#endif //SDV_PAINTBACKGROUNDFUNCTOR_H
