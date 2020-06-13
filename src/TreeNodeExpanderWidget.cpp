//
// Created by kca on 25/5/2020.
//

#include "TreeNodeExpanderWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>

namespace SDV {

// public explicit
TreeNodeExpanderWidget::
TreeNodeExpanderWidget(QWidget* parent /*= nullptr*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
    : Base{parent, f}, m_isExpanded{false}, m_isMouseOver{false}
{
    // Enable QEvent::Type::HoverEnter & HoverLeave
    setAttribute(Qt::WidgetAttribute::WA_Hover, true);
    setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
    setCursor(Qt::CursorShape::ArrowCursor);
}

// protected
void
TreeNodeExpanderWidget::
paintEvent(QPaintEvent* event)  // override
{
    QPainter painter{this};
    // Required: Without this setting, the paint looks terrible.
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    // Ref: https://doc.qt.io/qt-5/qpalette.html#ColorRole-enum
    // Text: "The foreground color used with Base. This is usually the same as the WindowText, in which case it must
    // provide good contrast with Window and Base."
    const QPalette& palette = this->palette();
    const QColor& colorText = palette.color(QPalette::ColorRole::Text);
    const QColor& colorDark = palette.color(QPalette::ColorRole::Dark);
    // I have a 4k monitor.  It seems like 3.0 pen width looks good.
    const qreal penWidth = 3.0;
    painter.setPen(QPen{m_isMouseOver ? colorText : colorDark, penWidth});
    painter.drawRect(rect());
    painter.setPen(QPen{colorText, penWidth});
    const qreal margin = 2.0 * penWidth;
    // Paint horizontal line (minus sign: '-')
    painter.drawLine(
        QLineF{
            QPointF{margin, height() / 2.0},
            QPointF{width() - margin, height() / 2.0}});
    // If we are collapsed, then paint vertical line to create plus sign: '+'
    if (false == m_isExpanded) {
        painter.drawLine(
            QLineF{
                QPointF{width() / 2.0, margin},
                QPointF{width() / 2.0, height() - margin}});
    }
}

// protected
bool
TreeNodeExpanderWidget::
event(QEvent* event)  // override
{
    const QEvent::Type type = event->type();
    switch (type) {
        case QEvent::Type::HoverEnter: {
            m_isMouseOver = true;
            break;
        }
        case QEvent::Type::HoverLeave: {
            m_isMouseOver = false;
            break;
        }
    }
    const bool x = Base::event(event);
    return x;
}

// protected
void TreeNodeExpanderWidget::
mousePressEvent(QMouseEvent* event)  // override
{
    Base::mousePressEvent(event);
    m_isExpanded = ! m_isExpanded;
    update();
    emit signalIsExpanded(m_isExpanded);
}

}  // namespace SDV
