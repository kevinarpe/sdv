//
// Created by kca on 15/4/2020.
//

#include "LineNumberAreaWidget.h"
#include "IWidgetWithLineNumberArea.h"

namespace SDV {

// public
QSize
LineNumberAreaWidget::
sizeHint()
const // override
{
    return QSize{m_widget.lineNumberAreaWidth(), 0};
}

// protected
void
LineNumberAreaWidget::
paintEvent(QPaintEvent* event)  // override
{
    m_widget.lineNumberAreaPaintEvent(event);
}

}  // namespace SDV
