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
#include "Algorithm.h"

namespace SDV {

static const QString SPACE_GRAPHEME{QLatin1Char{' '}};
// Baby blue borrowed from IntelliJ! :)
static const QBrush SELECTED_TEXT_BRUSH{QColor{166, 210, 255}};

// private
struct TextView::Private
{
    static qreal
    maxLineWidth(const std::vector<QString>& lineVec, const QFontMetricsF& fontMetricsF)
    {
        qreal x = 0;
        std::for_each(lineVec.begin(), lineVec.end(),
            [&fontMetricsF, &x](const QString& line)
            {
                x = std::max(x, fontMetricsF.horizontalAdvance(line));
            });
        return x;
    }

    struct SelectionRange
    {
        /** Inclusive and normalised for backward selection -- always less or equal to lastLineIndex. */
        const int firstLineIndex;
        /** Inclusive and normalised for backward selection -- always greater or equal to lastLineIndex. */
        const int lastLineIndex;

        explicit SelectionRange(const TextView& self)
            : firstLineIndex{std::min(self.m_textCursor->selection().begin.lineIndex,
                                      self.m_textCursor->selection().end.lineIndex)},
              lastLineIndex{std::max(self.m_textCursor->selection().begin.lineIndex,
                                     self.m_textCursor->selection().end.lineIndex)}
        {
            const TextViewSelection& selection = self.m_textCursor->selection();
            if (selection.begin.isValid())
            {
                assert(false == selection.begin.isEqual(selection.end));
            }
        }
    };

    struct LineSelection
    {
        /** QChar count before selection */
        int beforeLength;
        /** QChar count of selection */
        int length;
        /** QChar count after selection */
        int afterLength;
        /**
         * Does selection end on current line?
         * <br>If true, do not highlight to right edge of viewport.
         * <br>If false, highlight to right edge of viewport.
         */
        bool isEnd;

        static LineSelection none(const QString& line) {
            return LineSelection{.beforeLength = line.length(), .length = 0, .afterLength = 0, .isEnd = true};
        }
    };

    /**
     * @param line
     *        Technically, we only need line.length(), but the full text line is nice for debugging! :)
     */
    static LineSelection
    lineSelection(const TextView& self,
                  const SelectionRange& selectionRange,
                  const int lineIndex,
                  const QString& line)
    {
        const TextViewSelection& selection = self.textCursor().selection();
        if (selection.begin.isValid() == false
            || lineIndex < selectionRange.firstLineIndex
            || lineIndex > selectionRange.lastLineIndex)
        {
            const LineSelection& x = LineSelection::none(line);
            return x;
        }
        // Forward selection
        if (selection.begin.isLessThan(selection.end))
        {
            if (lineIndex == selection.begin.lineIndex)
            {
                if (lineIndex == selection.end.lineIndex)
                {
                    // Forward selection: First and last line are same.
                    const LineSelection x{
                        .beforeLength = selection.begin.charIndex,
                        .length = selection.end.charIndex - selection.begin.charIndex,
                        .afterLength = line.length() - selection.end.charIndex,
                        .isEnd = true
                    };
                    return x;
                }
                else if (lineIndex < selection.end.lineIndex)
                {
                    // Forward selection: First line, but more than one line.
                    const LineSelection x{
                        .beforeLength = selection.begin.charIndex,
                        .length = line.length() - selection.begin.charIndex,
                        .afterLength = 0,
                        .isEnd = false
                    };
                    return x;
                }
                else {
                    assert(false);
                }
            }
            else if (lineIndex < selection.end.lineIndex)
            {
                // Forward selection: Middle line, but not last.
                const LineSelection x{
                    .beforeLength = 0,
                    .length = line.length(),
                    .afterLength = 0,
                    .isEnd = false
                };
                return x;
            }
            else if (lineIndex == selection.end.lineIndex)
            {
                // Forward selection: Last line of many.
                const LineSelection x{
                    .beforeLength = 0,
                    .length = selection.end.charIndex,
                    .afterLength = line.length() - selection.end.charIndex,
                    .isEnd = true
                };
                return x;
            }
            else {
                assert(false);
            }
        }
        // Backward selection
        else {
            if (lineIndex == selection.end.lineIndex)
            {
                if (lineIndex == selection.begin.lineIndex)
                {
                    // Backward selection: First and last line are same.
                    const LineSelection x{
                        .beforeLength = selection.end.charIndex,
                        .length = selection.begin.charIndex - selection.end.charIndex,
                        .afterLength = line.length() - selection.begin.charIndex,
                        .isEnd = true
                    };
                    return x;
                }
                else if (lineIndex < selection.begin.lineIndex)
                {
                    // Backward selection: First line, but more than one line.
                    const LineSelection x{
                        .beforeLength = selection.end.charIndex,
                        .length = line.length() - selection.end.charIndex,
                        .afterLength = 0,
                        .isEnd = false
                    };
                    return x;
                }
                else {
                    assert(false);
                }
            }
            else if (lineIndex < selection.begin.lineIndex)
            {
                // Backward selection: Middle line, but not last.
                const LineSelection x{
                    .beforeLength = 0,
                    .length = line.length(),
                    .afterLength = 0,
                    .isEnd = false
                };
                return x;
            }
            else if (lineIndex == selection.begin.lineIndex)
            {
                // Backward selection: Last line of many.
                const LineSelection x{
                    .beforeLength = 0,
                    .length = selection.begin.charIndex,
                    .afterLength = line.length() - selection.begin.charIndex,
                    .isEnd = true
                };
                return x;
            }
            else {
                assert(false);
            }
        }
        assert(false);
    }

    static void
    paintTextCursor(TextView& self, QPainter& painter, const QFontMetricsF& fontMetricsF, const QPalette& palette)
    {
        const QString& grapheme = self.m_textCursor->grapheme();
        const bool isSpace = (SPACE_GRAPHEME == grapheme);
        // If text cursor is visible, always paint here.
        if (self.m_textCursor->isVisible())
        {
            painter.fillRect(self.m_textCursorRectF, QBrush{QColor{Qt::GlobalColor::black}});
            if (false == isSpace) {
                painter.setPen(QPen{QColor{Qt::GlobalColor::white}});
            }
        }
        // If text cursor is *not* visible, only re-paint when flagged for update.
        else if (self.m_textCursor->isUpdate())
        {
            const TextViewSelection& selection = self.m_textCursor->selection();

            // If selection is backward (right-to-left), then grapheme under text cursor will be selected.
            const bool isTextCursorSelected =
                selection.begin.isValid() && self.m_textCursor->pos().isLessThan(selection.begin);

            if (isTextCursorSelected) {
                painter.fillRect(self.m_textCursorRectF, SELECTED_TEXT_BRUSH);
            }
            else {
                painter.fillRect(self.m_textCursorRectF, QBrush{palette.color(QPalette::ColorRole::Base)});
            }

            if (false == isSpace) {
                painter.setPen(QPen{palette.color(QPalette::ColorRole::Text)});
            }
        }

        if (false == isSpace) {
            // Always paint text with QPointF, instead of QRectF.  Why?  CJK chars will not paint correctly.
            const qreal y = self.m_textCursorRectF.y() + fontMetricsF.ascent();
            painter.drawText(QPointF{self.m_textCursorRectF.x(), y}, grapheme);
        }

        self.m_textCursor->afterPaintEvent();
    }
};

// public explicit
TextView::
TextView(QWidget* parent /*= nullptr*/)
    : Base{parent},
      m_docView{std::make_shared<TextViewDocumentView>()},
      m_textCursor{std::make_unique<TextViewTextCursor>(*this, m_docView)},
      m_graphemeFinder{std::make_unique<GraphemeFinder>()},
      m_isAfterSetDoc{false},
      m_firstVisibleLineIndex{-1}, m_lastFullyVisibleLineIndex{-1}, m_lastVisibleLineIndex{-1}
{}

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
    m_textCursor->reset();
    m_isAfterSetDoc = true;
    viewport()->update();
}

// public
int
TextView::
lineIndexForHeight(const qreal viewportYCoord)
const
{
    const QFontMetricsF fontMetricsF{font()};
    const qreal lineSpacing = fontMetricsF.lineSpacing();
    const qreal stepCountF = viewportYCoord / lineSpacing;
    // truncate (round down)
    const int stepCount = static_cast<int>(stepCountF);
    // lineIndex <= m_lastVisibleLineIndex
    const int lineIndex = m_docView->nextVisibleLineIndex(m_firstVisibleLineIndex, stepCount);
    return lineIndex;
}

// public
//TextViewPosition
TextView::Position
TextView::
positionForPoint(const QPointF& viewportPointF, GraphemeFinder::IncludeTextCursor includeTextCursor)
const
{
    const int lineIndex = lineIndexForHeight(viewportPointF.y());
    const QString& line =
        Algorithm::Vector::valueOrDefault(m_docView->doc().lineVec(), lineIndex, QString{});

    const QFontMetricsF fontMetricsF{font()};
    const GraphemeFinder::Result r =
        m_graphemeFinder->positionForFontWidth(line, fontMetricsF, viewportPointF.x(), includeTextCursor);

    const Position& x = Position{.lineIndex = lineIndex, .grapheme = r};
    return x;
}

// protected
void
TextView::
paintEvent(QPaintEvent* event)  // override
{
    // Intentional: Do not call Base.  The impl is empty.
//    Base::paintEvent(event);

    const QFontMetricsF fontMetricsF{font()};
    const qreal lineSpacing = fontMetricsF.lineSpacing();
    const std::vector<QString>& textLineVec = m_docView->doc().lineVec();
    if (m_isAfterSetDoc)
    {
        m_isAfterSetDoc = false;

        QScrollBar* vbar = verticalScrollBar();
        // static_cast<int> will discard the final line if partially visible.
        const int fullyVisibleLineCount = static_cast<int>(viewport()->height() / lineSpacing);
        // vbar->value() is top line index.  The range is *inclusive*.
        vbar->setRange(0, m_docView->visibleLineCount() - fullyVisibleLineCount);
        vbar->setValue(0);
        vbar->setPageStep(fullyVisibleLineCount);

        // hbar->value() is 'negative pixel offset'.  If 345, then shift viewport 345 pixels left.
        QScrollBar* hbar = horizontalScrollBar();
        const qreal maxLineWidth = Private::maxLineWidth(textLineVec, fontMetricsF);
        hbar->setRange(0, qRound(maxLineWidth) - viewport()->width());
        hbar->setValue(0);
        hbar->setPageStep(width());
    }
    const QPalette& palette = this->palette();
    QPainter painter{viewport()};
    painter.setFont(font());
    const bool isOnlyTextCursorUpdate = m_textCursor->isUpdate() && event->rect() == m_textCursorRect;
    if (false == isOnlyTextCursorUpdate)
    {
        painter.fillRect(event->rect(), QBrush{palette.color(QPalette::ColorRole::Base)});

        if (0 == m_docView->visibleLineCount())
        {
            m_firstVisibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = -1;
        }
        else {
            // TODO: Create another class TextViewTextCursorView to track the *display* of TextViewTextCursor.
            // Why?  Decouple display logic.  Example: PageUp/Down tries to keep cursor in the same relative
            // location before and after move.

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
            const qreal x = -1 * horizontalScrollBar()->value();
            // Intentional: drawText() expects y as *bottom* of text line.
            qreal y = fontMetricsF.ascent();
            const Private::SelectionRange selectionRange = Private::SelectionRange{*this};

            auto visibleLineIndexIter = m_docView->visibleLineBegin() + verticalScrollBar()->value();
            m_firstVisibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = *visibleLineIndexIter;

            if (m_textCursor->hasMoved()) {
                // Intentional: Defensive programming :)
                m_textCursorRectF = QRectF{};
                m_textCursorRect = QRect{};
            }
            // TODO: ADD SUPPORT FOR TEXT FORMATTING
            int visibleLineOffset = 0;
            for ( ; visibleLineOffset < visibleLineCount; ++visibleLineOffset, ++visibleLineIndexIter)
            {
                if (m_docView->visibleLineEnd() == visibleLineIndexIter) {
                    break;
                }
                const int visibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = *visibleLineIndexIter;
                const QString& line = textLineVec[visibleLineIndex];
                const Private::LineSelection lineSelection =
                    Private::lineSelection(*this, selectionRange, visibleLineIndex, line);

                // If any selection exists on this line, only paint the background.
                if (lineSelection.length > 0)
                {
                    const QString beforeSelectedText = line.left(lineSelection.beforeLength);
                    const QString selectedText = line.mid(lineSelection.beforeLength, lineSelection.length);

                    const qreal beforeSelectedWidth = fontMetricsF.horizontalAdvance(beforeSelectedText);
                    const qreal selectedWidth = fontMetricsF.horizontalAdvance(selectedText);

                    const QRectF& r = QRectF{QPointF{x + beforeSelectedWidth, y - fontMetricsF.ascent()},
                                             QSizeF{selectedWidth, lineSpacing}};

                    painter.fillRect(r, SELECTED_TEXT_BRUSH);

                    if (lineSelection.isEnd == false)
                    {
                        const QString afterSelectedText = line.left(lineSelection.afterLength);
                        const qreal afterSelectedWidth = fontMetricsF.horizontalAdvance(afterSelectedText);
                        const qreal x2 = x + beforeSelectedWidth + selectedWidth + afterSelectedWidth;

                        if (viewport()->width() - x2 > 0.0)
                        {
                            const QRectF& r = QRectF{QPointF{x2, y - fontMetricsF.ascent()},
                                                     QSizeF{viewport()->width() - x2, lineSpacing}};

                            painter.fillRect(r, SELECTED_TEXT_BRUSH);
                        }
                    }
                }
                // From experience, the full line text should be drawn in a single call.  Why?  If we try to draw pieces
                // and carefully colour the background (for selection, etc.), there will be sub-pixel discrepancies.
                // If you "look under a microscope" (as I do!), it will drive you crazy.  Each move of the cursor will
                // shift before-/after-/selected text by sub-pixel horizontal distances.  Insane!

                // From Qt5 docs: "Note: The y-position is used as the baseline of the font."
                painter.drawText(QPointF{x, y}, line);

                y += lineSpacing;
                if (visibleLineIndex == m_textCursor->pos().lineIndex && m_textCursorRectF.isValid() == false)
                {
                    const qreal x2 = x + fontMetricsF.horizontalAdvance(line, m_textCursor->pos().charIndex);
                    const QString& grapheme = m_textCursor->grapheme();
                    const qreal width = fontMetricsF.horizontalAdvance(grapheme);
                    const qreal y2 = lineSpacing * visibleLineOffset;
                    m_textCursorRectF = QRectF{QPointF{x2, y2}, QSizeF{width, lineSpacing}};
                    m_textCursorRect = m_textCursorRectF.toRect();
                }
            }
            // Only adjust if we have a "full view": visibleLineCount == i
            if (visibleLineCount == visibleLineOffset && visibleLineCount > fullyVisibleLineCount) {
                --m_lastFullyVisibleLineIndex;
            }
        }
    }
    Private::paintTextCursor(*this, painter, fontMetricsF, palette);
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

    if (false == event->size().isValid()) {
        // Ignore resize even when new height or widget is negative.
        return;
    }
    // Tricky: event->oldSize() can be -1 on first resize.
    const int heightDiff = event->size().height() - std::max(0, event->oldSize().height());
    if (0 != heightDiff)
    {
        const QFontMetricsF fontMetricsF{font()};
        // static_cast<int> will discard the final line if partially visible.
        const int fullyVisibleLineCount = static_cast<int>(event->size().height() / fontMetricsF.lineSpacing());
        QScrollBar* const vbar = verticalScrollBar();
        vbar->setPageStep(fullyVisibleLineCount);
        const int pageStepDiff = fullyVisibleLineCount - vbar->pageStep();
        vbar->setMaximum(vbar->maximum() + pageStepDiff);
    }
    // Tricky: event->oldSize() can be -1 on first resize.
    const int widthDiff = event->size().width() - std::max(0, event->oldSize().width());
    if (0 != widthDiff)
    {
        QScrollBar* const hbar = horizontalScrollBar();
        hbar->setMaximum(hbar->maximum() + widthDiff);
        hbar->setPageStep(hbar->pageStep() + widthDiff);
    }
}

}  // namespace SDV
