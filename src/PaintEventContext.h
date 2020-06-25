//
// Created by kca on 25/6/2020.
//

#ifndef SDV_PAINTEVENTCONTEXT_H
#define SDV_PAINTEVENTCONTEXT_H

class QWidget;

namespace SDV {

struct PaintEventContext
{
    virtual ~PaintEventContext() = 0;
    virtual void update(const QWidget& widget) = 0;
};

}  // namespace SDV

#endif //SDV_PAINTEVENTCONTEXT_H
