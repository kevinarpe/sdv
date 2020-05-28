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
TreeNodeExpanderWidget(const QFont& font, QWidget* parent /*= nullptr*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
    : Base{parent, f}, m_isExpanded{false}, m_isMouseOver{false}
{
    setFont(font);
    setAttribute(Qt::WA_Hover, true);
    setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Fixed});
}

// public slot
void
TreeNodeExpanderWidget::
slotSetExpanded(const bool isExpanded)
{
    if (isExpanded != m_isExpanded) {
        m_isExpanded = isExpanded;
        emit signalIsExpanded(isExpanded);
    }
}

//LAST: LOOKS OK!
// protected
void
TreeNodeExpanderWidget::
paintEvent(QPaintEvent* event)  // override
{
    QPainter painter{this};
    // Required: Without this setting, the paint looks terrible.
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    // Ref: https://doc.qt.io/qt-5/qpalette.html#ColorRole-enum
    // Base: "Used mostly as the background color for text entry widgets, but can also be used for other painting -
    // such as the background of combobox drop down lists and toolbar handles. It is usually white or another light color."
    // Text: "The foreground color used with Base. This is usually the same as the WindowText, in which case it must
    // provide good contrast with Window and Base."
    const QPalette& palette = this->palette();
    const QColor& colorText = palette.color(QPalette::ColorRole::Text);
//    const QColor& colorWindow = palette.color(QPalette::ColorRole::Window);
//    const QColor& colorWindow = palette.color(QPalette::ColorRole::Button);
    const QColor& colorWindow = palette.color(QPalette::ColorRole::Dark);
    // I have a 4k monitor.  It seems like 3.0 pen width looks good.
    const qreal penWidth = 3.0;
    const QPoint& topLeft = QPoint{0, 0};
    if (true || m_isMouseOver) {
        painter.setPen(QPen{m_isMouseOver ? colorText : colorWindow, penWidth});
        painter.drawRect(QRect{topLeft, size()});
    }
    painter.setPen(QPen{colorText, penWidth});
    const qreal xpos = 6.0;
    // Paint minus sign: '-'
    painter.drawLine(
        QLineF{
            QPointF{xpos, height() / 2.0},
            QPointF{width() - xpos, height() / 2.0}});
    // If we are collapsed, then paint vertical line to create plus sign: '+'
    if (false == m_isExpanded) {
        painter.drawLine(
            QLineF{
                QPointF{width() / 2.0, xpos},
                QPointF{width() / 2.0, height() - xpos}});
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
//            if (!m_isMouseOver) {
//                qDebug() << "m_isMouseOver";
//            }
            m_isMouseOver = true;
            break;
        }
        case QEvent::Type::HoverLeave: {
//            if (m_isMouseOver) {
//                qDebug() << "!m_isMouseOver";
//            }
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
    m_isExpanded = !m_isExpanded;
    update();
    emit signalIsExpanded(m_isExpanded);
}

}  // namespace SDV
