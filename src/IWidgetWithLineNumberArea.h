//
// Created by kca on 15/4/2020.
//

#ifndef SDV_IWIDGETWITHLINENUMBERAREA_H
#define SDV_IWIDGETWITHLINENUMBERAREA_H

class QPaintEvent;

namespace SDV {

class IWidgetWithLineNumberArea
{
public:
    virtual ~IWidgetWithLineNumberArea() = default;

    virtual int lineNumberAreaWidth() const = 0;
    virtual void lineNumberAreaPaintEvent(QPaintEvent* event) = 0;
};

}  // namespace SDV

#endif //SDV_IWIDGETWITHLINENUMBERAREA_H
