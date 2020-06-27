//
// Created by kca on 25/6/2020.
//

#ifndef SDV_PAINTFOREGROUNDFUNCTOR_H
#define SDV_PAINTFOREGROUNDFUNCTOR_H

class QPainter;
#include "LineFormat.h"

namespace SDV {

class PaintContext;

struct PaintForegroundFunctor
{
    virtual ~PaintForegroundFunctor() = 0;

    /**
     * Call {@code QPainter::setFont(...)} and/or {@code QPainter::setPen(...)}
     */
    virtual void beforeDrawText(QPainter& painter, PaintContext* nullableContext) const = 0;
};

using LineFormatForeground = LineFormat<PaintForegroundFunctor>;

}  // namespace SDV

#endif //SDV_PAINTFOREGROUNDFUNCTOR_H
