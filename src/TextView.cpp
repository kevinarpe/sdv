//
// Created by kca on 6/6/2020.
//

#include "TextView.h"
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QPaintEvent>
#include <QApplication>
#include <QShortcut>
#include <algorithm>
#include <cmath>
#include "TextViewDocument.h"
#include "TextViewDocumentView.h"
#include "TextViewTextCursor.h"

namespace SDV {

// private
struct TextView::Private
{
    static void
    slotHorizontalScrollBarActionTriggered(TextView& self, QAbstractSlider::SliderAction action)
    {
        self.viewport()->update();
    }

    static void
    slotVerticalScrollBarActionTriggered(TextView& self, QAbstractSlider::SliderAction action)
    {
        self.viewport()->update();
    }

    static qreal
    maxLineWidth(const std::vector<QString>& lineVec, const QFontMetricsF& fontMetricsF)
    {
        qreal x = 0;
        std::for_each(lineVec.begin(), lineVec.end(),
            [&fontMetricsF, &x](const QString& line) {
                x = std::max(x, fontMetricsF.horizontalAdvance(line));
            });
        return x;
    }
//
//    static void
//    updateTextCursorRect(TextView& self, const QFontMetricsF& fontMetricsF)
//    {
//        const QChar& ch = getTextCursorChar(self);
//        const qreal width = fontMetricsF.horizontalAdvance(ch);
//        m_textCursorRectF = QRectF{QPointF{0, 0}, QSizeF{width, lineSpacing}};
//        m_textCursorRect = m_textCursorRectF.toRect();
//    }
};

// public explicit
TextView::
TextView(QWidget* parent /*= nullptr*/)
    : Base{parent},
      m_docView{std::make_unique<TextViewDocumentView>()},
      m_textCursor{std::make_unique<TextViewTextCursor>(*this, *m_docView)},
      m_isAfterSetDoc{false},
      m_firstVisibleLineIndex{-1}, m_lastFullyVisibleLineIndex{-1}, m_lastVisibleLineIndex{-1}
{
    QObject::connect(horizontalScrollBar(), &QScrollBar::actionTriggered,
        [this](int action) {
            Private::slotHorizontalScrollBarActionTriggered(*this, static_cast<QAbstractSlider::SliderAction>(action));
        });
    QObject::connect(verticalScrollBar(), &QScrollBar::actionTriggered,
        [this](int action) {
            Private::slotVerticalScrollBarActionTriggered(*this, static_cast<QAbstractSlider::SliderAction>(action));
        });
//    {
//        QShortcut* s = new QShortcut(QKeySequence(Qt::Key_Left), this);
//        QObject::connect(s, &QShortcut::activated, [this]() { Private::slotTextCursorLeft(*this); });
//    }
//    {
//        QShortcut* s = new QShortcut(QKeySequence(Qt::Key_Right), this);
//        QObject::connect(s, &QShortcut::activated, [this]() { Private::slotTextCursorRight(*this); });
//    }
}

// Intentional: Impl here b/c of forward decls in header.
// public
TextView::
~TextView() = default;  // override

// public
void
TextView::
setDoc(const std::shared_ptr<TextViewDocument>& doc)
{
    m_docView->setDoc(doc);
    m_isAfterSetDoc = true;
    viewport()->update();
}

// protected
void
TextView::
paintEvent(QPaintEvent* event)  // override
{
    // Intentional: Do not call Base.  The impl is empty.
//    Base::paintEvent(event);

    const QFontMetricsF& fontMetricsF = QFontMetricsF{font()};
    const qreal lineSpacing = fontMetricsF.lineSpacing();
    const std::vector<QString>& textLineVec = m_docView->doc().textLineVec();
    if (m_isAfterSetDoc)
    {
        m_isAfterSetDoc = false;

        QScrollBar* vbar = verticalScrollBar();
        // vbar->value() is top line index.  The range is *inclusive*.
        vbar->setRange(0, m_docView->visibleLineIndexSet().size() - 1);
        vbar->setValue(0);
        // static_cast<int> will discard the final line if partially visible.
        const int fullyVisibleLineCount = static_cast<int>(viewport()->height() / lineSpacing);
        vbar->setPageStep(fullyVisibleLineCount);

        // hbar->value() is 'negative pixel offset'.  If 345, then shift viewport 345 pixels left.
        QScrollBar* hbar = horizontalScrollBar();
        const qreal maxLineWidth = Private::maxLineWidth(textLineVec, fontMetricsF);
        hbar->setRange(0, qRound(maxLineWidth) - width());
        hbar->setValue(0);
        hbar->setPageStep(width());
    }
//    qDebug() << "TextView::paintEvent" << event->rect() << event->region();
    const QPalette& palette = this->palette();
    QPainter painter{viewport()};
    painter.setFont(font());
    const bool isOnlyTextCursorUpdate = m_textCursor->isUpdate() && event->rect() == m_textCursorRect;
    if (false == isOnlyTextCursorUpdate)
    {
        qDebug() << "TextView::paintEvent(text)" << event->rect();
        painter.fillRect(event->rect(), QBrush{palette.color(QPalette::ColorRole::Base)});

        if (m_docView->visibleLineIndexSet().empty())
        {
            m_firstVisibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = -1;
        }
        else {
            const std::size_t beginLineIndex = verticalScrollBar()->value();
            if (m_textCursor->hasMoved())
            {

            }
            // static_cast<int> will discard the final line if partially visible.
            const int fullyVisibleLineCount = static_cast<int>(viewport()->height() / lineSpacing);
            // Includes final line if partially visible.
            const int visibleLineCount = std::ceil(viewport()->height() / lineSpacing);
            assert(visibleLineCount >= 1
                   && visibleLineCount >= fullyVisibleLineCount
                   && visibleLineCount - fullyVisibleLineCount <= 1);

            painter.setFont(font());
            // TODO: How to add text formatting?
//            painter.setPen(QPen{palette.color(QPalette::ColorRole::Text)});
            painter.setPen(QPen{QColor{Qt::GlobalColor::black}});

            // If hbar->value() is positive, then shift left.
            const qreal x = -1.0 * horizontalScrollBar()->value();
            // Intentional: drawText() expects y as *bottom* of text line.
            qreal y = fontMetricsF.ascent();

            std::set<int>::const_iterator visibleLineIndexIter = m_docView->visibleLineIndexSet().find(beginLineIndex);
            assert(m_docView->visibleLineIndexSet().end() != visibleLineIndexIter);
            m_firstVisibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = *visibleLineIndexIter;

            if (m_textCursor->hasMoved()) {
                m_textCursorPrevRectF = m_textCursorRectF;
                // Intentional: Defensive programming :)
                m_textCursorRectF = QRectF{};
                m_textCursorRect = QRect{};
            }
            int i = 0;
            for ( ; i < visibleLineCount; ++i, ++visibleLineIndexIter) {
                if (m_docView->visibleLineIndexSet().end() == visibleLineIndexIter) {
                    break;
                }
                const int visibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = *visibleLineIndexIter;
                const QString& line = textLineVec[visibleLineIndex];
                // From Qt5 docs: "Note: The y-position is used as the baseline of the font."
                painter.drawText(QPointF{x, y}, line);
                y += lineSpacing;
                // TODO: Can we simplify first line in this condition?  See "Defensive programming" above.
                if ((false == m_textCursorRectF.isValid() || m_textCursor->hasMoved())
                    && visibleLineIndex == m_textCursor->lineIndex())
                {
                    const qreal x = fontMetricsF.horizontalAdvance(line, m_textCursor->columnIndex());
                    const QChar& ch = m_textCursor->ch();
                    const qreal width = fontMetricsF.horizontalAdvance(ch);
                    const qreal y = lineSpacing * m_textCursor->lineIndex();
                    m_textCursorRectF = QRectF{QPointF{x, y}, QSizeF{width, lineSpacing}};
                    m_textCursorRect = m_textCursorRectF.toRect();
                }
            }
            // Only adjust if we have a "full view": visibleLineCount == i
            if (visibleLineCount == i && visibleLineCount > fullyVisibleLineCount) {
                --m_lastFullyVisibleLineIndex;
            }
        }
    }
//    if (m_textCursor->hasMoved())
//    {
//        if (m_textLineVec.empty())
//        {
//            const qreal width = fontMetricsF.horizontalAdvance(QLatin1Char{' '});
//            m_textCursorRectF = QRectF{QPointF{0, 0}, QSizeF{width, lineSpacing}};
//        }
//        else {
//            const QString& line = m_textLineVec[m_textCursorLineIndex];
//            const qreal x = fontMetricsF.horizontalAdvance(line, m_textCursorColumnIndex);
//            const QChar& ch = Private::getTextCursorChar(*this);
//            const qreal width = fontMetricsF.horizontalAdvance(ch);
//            const qreal y = lineSpacing * m_textCursorLineIndex;
//            m_textCursorRectF = QRectF{QPointF{x, y}, QSizeF{width, lineSpacing}};
////            const QChar& ch = Private::getTextCursorChar(*this);
////            const qreal width = fontMetricsF.horizontalAdvance(ch);
//        }
//        m_textCursorRect = m_textCursorRectF.toRect();
//    }
    const QChar& ch = m_textCursor->ch();
    const bool isSpace = ch.isSpace();
    // If text cursor is visible, always paint here.
    if (m_textCursor->isVisible())
    {
//        qDebug() << "m_textCursor->isVisible()" << m_textCursorRectF << ch;
        painter.fillRect(m_textCursorRectF, QBrush{QColor{Qt::GlobalColor::black}});
        if (false == isSpace) {
            painter.setPen(QPen{QColor{Qt::GlobalColor::white}});
        }
    }
    // If text cursor is *not* visible, only re-paint when flagged for update.
    else if (m_textCursor->isUpdate())
    {
//        qDebug() << "isTextCursorUpdate" << m_textCursorRectF << ch;
        painter.fillRect(m_textCursorRectF, QBrush{palette.color(QPalette::ColorRole::Base)});
        if (false == isSpace) {
            painter.setPen(QPen{palette.color(QPalette::ColorRole::Text)});
        }
    }
    if (false == isSpace) {
//        qDebug() << "painter.drawText" << m_textCursorRectF << ch;
        painter.drawText(m_textCursorRectF, ch);
    }
    m_textCursorPrevRectF = QRectF{};
    m_textCursor->afterPaintEvent();
    qDebug() << "";
}

/**
 * @param event
 *        parent widget is always {@link #viewport()}
 */
// protected
void
TextView::
resizeEvent(QResizeEvent* event)  // override
{
    // Intentional: Do not call Base.  The impl is empty.
//    Base::resizeEvent(event);

//    qDebug() << "TextView::resizeEvent" << event->oldSize() << "->" << event->size() << "vp:" << viewport()->size();

    if (false == event->size().isValid()) {
        // Ignore resize even when new height or widget is negative.
        return;
    }
    // Tricky: event->oldSize() can be -1 on first resize.
    const int heightDiff = event->size().height() - std::max(0, event->oldSize().height());
    if (0 != heightDiff)
    {
        const QFontMetricsF& fontMetricsF = QFontMetricsF{font()};
        // static_cast<int> will discard the final line if partially visible.
        const int fullyVisibleLineCount = static_cast<int>(event->size().height() / fontMetricsF.lineSpacing());
        verticalScrollBar()->setPageStep(fullyVisibleLineCount);
    }
    // Tricky: event->oldSize() can be -1 on first resize.
    const int widthDiff = event->size().width() - std::max(0, event->oldSize().width());
    if (0 != widthDiff)
    {
        QScrollBar* hbar = horizontalScrollBar();
        hbar->setMaximum(hbar->maximum() + widthDiff);
        hbar->setPageStep(hbar->pageStep() + widthDiff);
    }
}

}  // namespace SDV
