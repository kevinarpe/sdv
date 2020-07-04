//
// Created by kca on 21/6/2020.
//

#include "TextViewLineNumberArea.h"
#include <cmath>
#include <QScrollBar>
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QResizeEvent>
#include "TextViewTextCursor.h"
#include "TextViewDocumentView.h"
#include "TextViewDocument.h"

namespace SDV {

// public static
const qreal TextViewLineNumberArea::kDefaultLeftMarginCharWidthRatio = 0.5;

// public static
const qreal TextViewLineNumberArea::kDefaultRightMarginCharWidthRatio = 1.0;

// Again: Borrowed from IntelliJ.  :)
// public static
const QPen TextViewLineNumberArea::kDefaultTextPen{QColor{127, 129, 126}};

struct TextViewLineNumberArea::Private
{
    static void
    slotVisibleLineIndicesChanged(TextViewLineNumberArea& self)
    {
        updateWidth(self);
        // We always want to redraw everything after visible line indices change.
        self.update();
    }

    static void
    slotTextCursorLineChanged(TextViewLineNumberArea& self, const int lineIndex)
    {
        // TODO: Can we update *less* than everything?
        self.update();
    }

    static void
    updateWidth(TextViewLineNumberArea& self)
    {
        const int width = calcWidth(self);
        // Important: During init, -1 == self.m_width
        if (width != self.m_width)
        {
            self.m_width = width;
            self.updateGeometry();
        }
    }

    static int
    calcWidth(const TextViewLineNumberArea& self)
    {
        const int maxLineIndex = self.m_textView.lastVisibleLineIndex();
        assert(maxLineIndex >= 0);
        // +1?  Remember: log10 of 150 is 2, not 3. :)
        const qreal digitCountF = 1.0 + static_cast<int>(std::log10(1 + maxLineIndex));
        const QFontMetricsF fontMetricsF{self.m_textView.font()};
        const qreal charWidth = calcCharWidth(fontMetricsF);
        // +1.5?  Right margin is one char wide.
        const qreal y = charWidth * (kDefaultLeftMarginCharWidthRatio + digitCountF + kDefaultRightMarginCharWidthRatio);
        const int x = qRound(y);
        return x;
    }

    static qreal
    calcCharWidth(const QFontMetricsF& fontMetricsF)
    {
        const qreal x = fontMetricsF.horizontalAdvance(QLatin1Char{'9'});
        return x;
    }

    static void
    setClipRect(TextViewLineNumberArea& self, QPaintEvent* event, QPainter* painter)
    {
        QRect rect = event->rect();
        const int adjEventBottom = getAdjEventBottom(self, event);
        if (rect.bottom() > adjEventBottom)
        {
            rect.setBottom(adjEventBottom);
            painter->setClipRect(rect);
        }
    }

    static int
    getAdjEventBottom(TextViewLineNumberArea& self, QPaintEvent* event)
    {
        int eventBottom = event->rect().bottom();

        if (self.m_textView.horizontalScrollBar()->isVisible())
        {
            const QPoint& topLeft = QPoint{0, 0};
            const QPoint& hbarPosG = self.m_textView.horizontalScrollBar()->mapToGlobal(topLeft);
            // Important: Use m_lineNumberAreaWidget, not this, b/c event is relative to m_lineNumberAreaWidget.
            const QPoint& hbarPosG2 = self.mapFromGlobal(hbarPosG);
            if (eventBottom >= hbarPosG2.y())
            {
                // Remember: hbarPosG2.y() is the *first* vertical pixel of hbar.  We want to exclude this vertical pixel.
                eventBottom = hbarPosG2.y() - 1;
            }
        }
        return eventBottom;
    }
};

// public explicit
TextViewLineNumberArea::
TextViewLineNumberArea(TextView& textView,
                       QWidget* parent,  // = nullptr
                       Qt::WindowFlags f  // = Qt::WindowFlags()
                       )
  : Base{parent, f},
    m_textView{textView},
    m_leftMarginCharWidthRatio(kDefaultLeftMarginCharWidthRatio),
    m_rightMarginCharWidthRatio(kDefaultRightMarginCharWidthRatio),
    m_textPen(kDefaultTextPen)
{
    {
        const QFontMetricsF fontMetricsF{m_textView.font()};
        // 1.0?  By default, only leave space for line numbers with only one digit.
        const qreal charWidth = Private::calcCharWidth(fontMetricsF);
        m_width = charWidth * (kDefaultLeftMarginCharWidthRatio + 1.0 + kDefaultRightMarginCharWidthRatio);
    }
    QObject::connect(&textView, &TextView::signalVisibleLinesChanged,
                     [this]() { Private::slotVisibleLineIndicesChanged(*this); });

    QObject::connect(&textView.textCursor(), &TextViewTextCursor::signalLineChange,
                     [this](const int lineIndex) { Private::slotTextCursorLineChanged(*this, lineIndex); });
}

// public
QSize
TextViewLineNumberArea::
sizeHint()
const  // override
{
    return QSize{m_width, 0};
}

// public
void
TextViewLineNumberArea::
setLeftMarginCharWidthRatio(const qreal leftMarginCharWidthRatio)
{
    if (leftMarginCharWidthRatio != m_leftMarginCharWidthRatio)
    {
        m_leftMarginCharWidthRatio = leftMarginCharWidthRatio;
        Private::updateWidth(*this);
    }
}

// public
void
TextViewLineNumberArea::
setRightMarginCharWidthRatio(const qreal rightMarginCharWidthRatio)
{
    if (rightMarginCharWidthRatio != m_rightMarginCharWidthRatio)
    {
        m_rightMarginCharWidthRatio = rightMarginCharWidthRatio;
        Private::updateWidth(*this);
    }
}

// public
void
TextViewLineNumberArea::
setPen(const QPen& pen)
{
    if (pen != m_textPen)
    {
        m_textPen = pen;
        update();
    }
}

// TODO: IMPL WIDGET SCROLL IF POSSIBLE -- LOOKS VERY DIFFICULT!
// Why?  TextView::signalVisibleLinesChanged
// In theory, we would need "tighter" signalling that would scroll when possible, else do full repaint.

// protected
void
TextViewLineNumberArea::
paintEvent(QPaintEvent* event)  // override
{
    const QRect& eventRect = event->rect();
    const QRectF eventRectF{eventRect};
    QPainter painter{this};
    const QPalette& palette = this->palette();
    const QBrush& bgBrush = QBrush{palette.color(QPalette::ColorRole::Window)};
    painter.fillRect(eventRect, bgBrush);
    const bool isTextCursorLineBgColorEnabled = (bgBrush != m_textView.textCursorLineBackgroundBrush());

    painter.setPen(m_textPen);
    painter.setFont(m_textView.font());

    const QFontMetricsF fontMetricsF{m_textView.font()};
    const qreal lineSpacing = fontMetricsF.lineSpacing();
    const qreal x = 0;
    // The "big" assumption here: The "viewport" of QAbstractScrollArea is a direct child.
    // As a result, y() represents a coordinate system that is similar to this widget.
    const qreal y = m_textView.viewport()->y();
    const qreal charWidth = Private::calcCharWidth(fontMetricsF);
    // Note: The paint code below uses Qt::AlignmentFlag::AlignRight, so left margin is handled implicitly.
    // Right margin is one char wide.
    const qreal width = this->width() - (m_rightMarginCharWidthRatio * charWidth);
    // Note: If font leading is not zero, we *may* want to use font height here, instead of line spacing.
    QRectF drawTextRect{QPointF{x, y}, QSizeF{width, lineSpacing}};
    Private::setClipRect(*this, event, &painter);
    const TextViewGraphemePosition& pos = m_textView.textCursor().position();
    const std::vector<int>& visibleLineIndexVec = m_textView.docView().visibleLineIndexVec();
    const int firstVisibleLineIndex = m_textView.firstVisibleLineIndex();
    std::vector<int>::const_iterator iter = m_textView.docView().findOrAssert(firstVisibleLineIndex);

    for ( ; visibleLineIndexVec.end() != iter; ++iter)
    {
        const int lineIndex = *iter;
        const QString& number = QString::number(1 + lineIndex);
        // eventRectF may be less than whole widget.  If so, only repaint necessary area.
        if (eventRectF.intersects(drawTextRect))
        {
            if (isTextCursorLineBgColorEnabled && lineIndex == pos.pos.lineIndex)
            {
                // Note: If font leading is not zero, we *may* want to use font height here, instead of line spacing.
                QRectF r{drawTextRect.topLeft(), QSizeF{qreal(this->width()), lineSpacing}};
                painter.fillRect(r, m_textView.textCursorLineBackgroundBrush());
            }
            painter.drawText(drawTextRect, Qt::AlignmentFlag::AlignRight, number);
        }
        if (drawTextRect.bottom() >= height()) {
            break;
        }
        drawTextRect.moveTop(drawTextRect.top() + lineSpacing);
    }
}

}  // namespace SDV
