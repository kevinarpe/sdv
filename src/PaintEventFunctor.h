//
// Created by kca on 24/6/2020.
//

#ifndef SDV_PAINTEVENTFUNCTOR_H
#define SDV_PAINTEVENTFUNCTOR_H

#include <memory>
class QWidget;
class QPaintEvent;
class QPainter;
class QString;
class QRectF;

namespace SDV {

class PaintEventContext;

struct PaintEventFunctor
{
    virtual ~PaintEventFunctor() = 0;

    // Ideas: paintRect() -> bg & border, setFont(), setPen() -> font color
    virtual void operator()(QWidget& widget,
                            // @Nullable
                            PaintEventContext* context,
                            QPaintEvent& event,
                            QPainter& painter,
//                            const QString& text,
                            const QRectF& textBoundingRect) = 0;
};

}  // namespace SDV

#endif //SDV_PAINTEVENTFUNCTOR_H
