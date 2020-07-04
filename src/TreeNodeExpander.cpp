//
// Created by kca on 25/5/2020.
//

#include "TreeNodeExpander.h"
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QGuiApplication>

namespace SDV {

// public explicit
TreeNodeExpander::
TreeNodeExpander(QWidget* parent /*= nullptr*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
    : Base{parent, f}, m_isExpanded{false}, m_isMouseOver{false}
{
    // Enable QEvent::Type::HoverEnter & HoverLeave
    setAttribute(Qt::WidgetAttribute::WA_Hover, true);
    setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
    setCursor(Qt::CursorShape::ArrowCursor);
}

// public slot
void
TreeNodeExpander::
slotSetExpanded(const bool isExpanded)
{
    if (isExpanded == m_isExpanded) {
        return;
    }
    m_isExpanded = isExpanded;
    update();
    emit signalExpandedChanged(isExpanded);
}

// protected
void
TreeNodeExpander::
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
    const QColor& colorDark = palette.color(QPalette::ColorRole::Mid);
    const qreal borderWidth = 2.0;
    const QColor& borderColor = m_isMouseOver ? colorText : colorDark;
    painter.setPen(QPen{borderColor, borderWidth});
    painter.drawEllipse(QRectF{QPointF{borderWidth / 2.0, borderWidth / 2.0},
                                 QSizeF{width() - borderWidth, height() - borderWidth}});
    const qreal lineWidth = 3.0;
    painter.setPen(QPen{colorText, lineWidth});
    const qreal margin = 2.0 * lineWidth;
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
TreeNodeExpander::
event(QEvent* event)  // override
{
    const QEvent::Type type = event->type();
    switch (type) {
        case QEvent::Type::Show:
        case QEvent::Type::Hide:
        // I never knew about ShowToParent or HideToParent before debugging weird show/hide issues!
        case QEvent::Type::ShowToParent:
        case QEvent::Type::HideToParent:
        case QEvent::Type::HoverLeave: {
            m_isMouseOver = false;
            break;
        }
        case QEvent::Type::HoverEnter: {
            m_isMouseOver = true;
            break;
        }
    }
    const bool x = Base::event(event);
    return x;
}

// protected
void
TreeNodeExpander::
mousePressEvent(QMouseEvent* event)  // override
{
    // Intentional: Do not run base method.  Why?  Do not propagate.
//    Base::mousePressEvent(event);

    slotSetExpanded( ! m_isExpanded);
}

}  // namespace SDV
