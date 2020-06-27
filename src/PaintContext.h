//
// Created by kca on 26/6/2020.
//

#ifndef SDV_PAINTCONTEXT_H
#define SDV_PAINTCONTEXT_H

class QWidget;

namespace SDV {

struct PaintContext
{
    virtual ~PaintContext() = 0;
    virtual void update(const QWidget& widget) = 0;
};

}  // namespace SDV

#endif //SDV_PAINTCONTEXT_H
