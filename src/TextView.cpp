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
#include "TextViewGraphemePosition.h"
#include "Algorithm.h"
#include "PaintBackgroundFunctor.h"
#include "PaintForegroundFunctor.h"
#include "PaintContext.h"

namespace SDV {

// public static
const QPen TextView::kDefaultTextPen{QColor{Qt::GlobalColor::black}};

// public static
const QBrush TextView::kDefaultSelectedTextBackgroundBrush{QColor{166, 210, 255}};

// public static
//const QBrush TextView::kDefaultTextCursorLineBackgroundBrush{QColor{252, 250, 237}};  // HSV->S=6
const QBrush TextView::kDefaultTextCursorLineBackgroundBrush{QColor{252, 249, 229}};  // HSV->S=9
//const QBrush TextView::kDefaultTextCursorLineBackgroundBrush{QColor{252, 248, 222}};  // HSV->S=12

// private
struct TextView::Private
{
    static void
    updateVisibleLineCounts(TextView& self)
    {
        const QFontMetricsF fontMetricsF{self.font()};
        const qreal lineSpacing = fontMetricsF.lineSpacing();
        // static_cast<int> will discard the final line if partially visible.
        self.m_viewportFullyVisibleLineCount = static_cast<int>(self.viewport()->height() / lineSpacing);
        // Includes final line if partially visible.
        self.m_viewportVisibleLineCount = std::ceil(self.viewport()->height() / lineSpacing);
    }

    static qreal
    maxLineWidth(const TextView& self, const QFontMetricsF& fontMetricsF)
    {
        const std::vector<QString>& lineVec = self.m_docView->doc().lineVec();
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

        bool isValid() const { return (firstLineIndex >= 0 && lastLineIndex >= 0); }

        bool
        contains(const int lineIndex)
        const
        {
            const bool x = (isValid() && lineIndex >= firstLineIndex && lineIndex <= lastLineIndex);
            return x;
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
        if (false == selection.isValid() || false == selectionRange.contains(lineIndex))
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
    paintTextCursor(const TextView& self, QPainter& painter, const QFontMetricsF& fontMetricsF, const QPalette& palette)
    {
        const TextViewGraphemePosition& pos = self.m_textCursor->position();
        if (pos.pos.lineIndex < self.m_firstVisibleLineIndex || pos.pos.lineIndex > self.m_lastFullyVisibleLineIndex) {
            return;
        }
        const bool isEndOfLine = isTextCursorEndOfLine(self);

        // If text cursor is visible, always paint here.
        if (self.m_textCursor->isVisible())
        {
            painter.fillRect(self.m_textCursorRectF, QBrush{QColor{Qt::GlobalColor::black}});
            if (false == isEndOfLine) {
                painter.setPen(QPen{QColor{Qt::GlobalColor::white}});
            }
        }
        // If text cursor is *not* visible, only re-paint when flagged for update.
        else if (self.m_textCursor->isUpdate())
        {
            const TextViewSelection& selection = self.m_textCursor->selection();

            // If selection is backward (right-to-left), then grapheme under text cursor will be selected.
            const bool isTextCursorSelected = selection.begin.isValid() && pos.pos.isLessThan(selection.begin);

            if (isTextCursorSelected) {
                painter.fillRect(self.m_textCursorRectF, self.m_selectedTextBackgroundBrush);
            }
            else {
                painter.fillRect(self.m_textCursorRectF, QBrush{palette.color(QPalette::ColorRole::Base)});
            }

            if (false == isEndOfLine) {
                painter.setPen(QPen{palette.color(QPalette::ColorRole::Text)});
            }
        }

        if (false == isEndOfLine) {
            // Always paint text with QPointF, instead of QRectF.  Why?  CJK chars will not paint correctly.
            const qreal y = self.m_textCursorRectF.y() + fontMetricsF.ascent();
            painter.drawText(QPointF{self.m_textCursorRectF.x(), y}, pos.grapheme);
        }
        self.m_textCursor->afterPaintEvent();
    }

    static bool
    isTextCursorEndOfLine(const TextView& self)
    {
        const std::vector<QString>& lineVec = self.m_docView->doc().lineVec();
        if (lineVec.empty()) {
            return true;
        }
        const TextViewGraphemePosition& pos = self.m_textCursor->position();
        const QString& line = lineVec[pos.pos.lineIndex];
        const bool x = line.length() == pos.pos.charIndex;
        return x;
    }
};

// public explicit
TextView::
TextView(QWidget* parent /*= nullptr*/)
    : Base{parent},
      m_docView{std::make_shared<TextViewDocumentView>()},
      m_textCursor{std::make_unique<TextViewTextCursor>(*this, m_docView)},
      m_graphemeFinder{std::make_unique<GraphemeFinder>()},
      m_textPen{kDefaultTextPen},
      m_selectedTextBackgroundBrush{kDefaultSelectedTextBackgroundBrush},
      m_textCursorLineBackgroundBrush{kDefaultTextCursorLineBackgroundBrush},
      m_isAfterSetDoc{false},
      m_viewportFullyVisibleLineCount{0}, m_viewportVisibleLineCount{0},
      m_firstVisibleLineIndex{0}, m_lastFullyVisibleLineIndex{0}, m_lastVisibleLineIndex{0}
{}

// Required when using std::unique_ptr<> with forward declarations.
// Ref: https://stackoverflow.com/a/6089065/257299
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
void
TextView::
setTextPen(const QPen& pen)
{
    if (pen != m_textPen)
    {
        m_textPen = pen;
        update();
    }
}

// public
void
TextView::
setSelectedTextBackgroundBrush(const QBrush& brush)
{
    if (brush != m_selectedTextBackgroundBrush)
    {
        m_selectedTextBackgroundBrush = brush;
        update();
        // Intentional: Update line number area widget
        emit m_textCursor->signalLineChange(m_textCursor->position().pos.lineIndex);
    }
}

// public
void
TextView::
setTextCursorLineBackgroundBrush(const QBrush& brush)
{
    if (brush != m_textCursorLineBackgroundBrush)
    {
        m_textCursorLineBackgroundBrush = brush;
        update();
        // Intentional: Update line number area widget
        emit m_textCursor->signalLineChange(m_textCursor->position().pos.lineIndex);
    }
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
positionForPoint(const QPointF& viewportPointF,
                 GraphemeFinder::IncludeTextCursor includeTextCursor)
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
    // TODO: Can this be moved inside resizeEvent()?  setDoc() should only reset h/v values to zero.
    if (m_isAfterSetDoc)
    {
        m_isAfterSetDoc = false;

        QScrollBar* vbar = verticalScrollBar();
        // static_cast<int> will discard the final line if partially visible.
        const int fullyVisibleLineCount = static_cast<int>(viewport()->height() / lineSpacing);
        // vbar->value() is top line index.  The range is *inclusive*.
        vbar->setRange(0, m_docView->visibleLineIndexVec().size() - fullyVisibleLineCount);
        vbar->setValue(0);
        vbar->setPageStep(fullyVisibleLineCount);

        // hbar->value() is 'negative pixel offset'.  If 345, then shift viewport 345 pixels left.
        QScrollBar* hbar = horizontalScrollBar();
        const qreal maxLineWidth = Private::maxLineWidth(*this, fontMetricsF);
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
        const QBrush& bgBrush = QBrush{palette.color(QPalette::ColorRole::Base)};
        painter.fillRect(event->rect(), bgBrush);
        const bool isTextCursorLineBgColorEnabled = (bgBrush != m_textCursorLineBackgroundBrush);

        if (m_docView->visibleLineIndexVec().empty())
        {
            m_firstVisibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = -1;
        }
        // TODO: Add basic context menu with copy/Ctrl+C, select all/Ctrl+A, deselect/Ctrl+Shift+A
        else {
            // static_cast<int> will discard the final line if partially visible.
            const int fullyVisibleLineCount = static_cast<int>(viewport()->height() / lineSpacing);
            // Includes final line if partially visible.
            const int visibleLineCount = std::ceil(viewport()->height() / lineSpacing);
            assert(visibleLineCount >= 1
                   && visibleLineCount >= fullyVisibleLineCount
                   && visibleLineCount - fullyVisibleLineCount <= 1);

            painter.setFont(font());
            painter.setPen(m_textPen);

            // If hbar->value() is positive, then shift left.
            const qreal x = -1 * horizontalScrollBar()->value();
            // Intentional: drawText() expects y as *bottom* of text line.
            qreal y = fontMetricsF.ascent();
            const Private::SelectionRange selectionRange = Private::SelectionRange{*this};
            const std::vector<QString>& lineVec = m_docView->doc().lineVec();
            const TextViewGraphemePosition& pos = m_textCursor->position();
            bool isFirstPaintBackground = true;
            bool isFirstPaintForeground = true;
            const std::vector<int>& visibleLineIndexVec = m_docView->visibleLineIndexVec();
            auto visibleLineIndexIter = visibleLineIndexVec.begin() + verticalScrollBar()->value();
            const int first = m_firstVisibleLineIndex;
            const int lastFull = m_lastFullyVisibleLineIndex;
            const int last = m_lastVisibleLineIndex;
            m_firstVisibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = *visibleLineIndexIter;

            if (m_textCursor->hasMoved()) {
                // Intentional: Defensive programming :)
                m_textCursorRectF = QRectF{};
                m_textCursorRect = QRect{};
            }
            int visibleLineOffset = 0;
            for ( ;
                visibleLineOffset < visibleLineCount && visibleLineIndexVec.end() != visibleLineIndexIter;
                ++visibleLineOffset, ++visibleLineIndexIter)
            {
                const int visibleLineIndex = m_lastFullyVisibleLineIndex = m_lastVisibleLineIndex = *visibleLineIndexIter;
                const QString& line = lineVec[visibleLineIndex];

                if (visibleLineIndex == pos.pos.lineIndex && m_textCursorRectF.isValid() == false)
                {
                    const qreal x2 = x + fontMetricsF.horizontalAdvance(line, pos.pos.charIndex);
                    const qreal width = fontMetricsF.horizontalAdvance(pos.grapheme);
                    const qreal y2 = lineSpacing * visibleLineOffset;
                    m_textCursorRectF = QRectF{QPointF{x2, y2}, QSizeF{width, lineSpacing}};
                    m_textCursorRect = m_textCursorRectF.toRect();
                }
                // NOTES
                // Decouple:
                // (1) drawRect() -> fills bg (QBrush), draws optional border (QPen)
                // (2) drawText() -> draw text (QPen & QFont)
                // Background order:
                // (1) Text cursor line
                // (2) Text formatting (JSON, XML, HTML, etc.)
                // (3) Text select *or* Find highlight
                // Foreground order:
                // (1) Text formatting (JSON, XML, HTML, etc.)
                // Paint order:
                // (1) Whole line background
                // (2) Whole line foreground (text)

                // From experience, the background (and borders) should be drawn first, then text.  Why?  Sub-pixel
                // discrepancies will blemish segments.

                // Draw text cursor line (background)
                if (isTextCursorLineBgColorEnabled && pos.pos.lineIndex == visibleLineIndex)
                {
                    // Intentional: Add x because x <= 0.
                    const qreal width = viewport()->width() + x;
                    const QRectF& rectF = QRectF{QPointF{x, y - fontMetricsF.ascent()},
                                                 QSizeF{width, lineSpacing}};

                    painter.fillRect(rectF, m_textCursorLineBackgroundBrush);
                }
                // Draw background (and borders)
                {
                    auto iter = m_lineIndex_To_BackgroundFormatSet_Map.find(visibleLineIndex);
                    if (m_lineIndex_To_BackgroundFormatSet_Map.end() != iter && iter->second.empty() == false)
                    {
                        if (isFirstPaintBackground)
                        {
                            isFirstPaintBackground = false;
                            if (m_paintBackgroundContext) {
                                m_paintBackgroundContext->update(*this);
                            }
                        }
                        QRectF rectF = QRectF{QPointF{x, y - fontMetricsF.ascent()},
                                              QSizeF{qreal(viewport()->width()), lineSpacing}};

                        const BackgroundFormatSet& formatSet = iter->second;
                        for (const LineFormatBackground& format : formatSet)
                        {
                            QRectF textBoundingRectF{rectF};
                            textBoundingRectF.moveLeft(fontMetricsF.horizontalAdvance(line, format.seg.charIndex));
                            const QString& text = line.mid(format.seg.charIndex, format.seg.length);
                            textBoundingRectF.setWidth(fontMetricsF.horizontalAdvance(text));
                            painter.save();
                            format.f->draw(painter, m_paintBackgroundContext.get(), textBoundingRectF);
                            painter.restore();
                        }
                    }
                }
                // Draw text selection (background)
                const Private::LineSelection lineSelection =
                    Private::lineSelection(*this, selectionRange, visibleLineIndex, line);

                // If any selection exists on this line, only paint the background.
                if (lineSelection.length > 0)
                {
                    const QString beforeSelectedText = line.left(lineSelection.beforeLength);
                    const QString selectedText = line.mid(lineSelection.beforeLength, lineSelection.length);

                    const qreal beforeSelectedWidth = fontMetricsF.horizontalAdvance(beforeSelectedText);
                    const qreal selectedWidth = fontMetricsF.horizontalAdvance(selectedText);

                    const QRectF& rectF =
                        QRectF{QPointF{x + beforeSelectedWidth, y - fontMetricsF.ascent()},
                               QSizeF{selectedWidth, lineSpacing}};

                    painter.fillRect(rectF, m_selectedTextBackgroundBrush);

                    if (lineSelection.isEnd == false)
                    {
                        const QString afterSelectedText = line.left(lineSelection.afterLength);
                        const qreal afterSelectedWidth = fontMetricsF.horizontalAdvance(afterSelectedText);
                        const qreal x2 = x + beforeSelectedWidth + selectedWidth + afterSelectedWidth;

                        if (viewport()->width() - x2 > 0.0)
                        {
                            // Intentional: Add x2 because x2 <= 0.
                            const qreal width = viewport()->width() + x2;
                            const QRectF& rectF = QRectF{QPointF{x2, y - fontMetricsF.ascent()},
                                                         QSizeF{width, lineSpacing}};

                            painter.fillRect(rectF, m_selectedTextBackgroundBrush);
                        }
                    }
                }
                // Draw text
                {
                    auto iter = m_lineIndex_To_ForegroundFormatSet_Map.find(visibleLineIndex);
                    if (m_lineIndex_To_ForegroundFormatSet_Map.end() == iter || iter->second.empty())
                    {
                        // From Qt5 docs: "Note: The y-position is used as the baseline of the font."
                        painter.drawText(QPointF{x, y}, line);
                    }
                    else {
                        if (isFirstPaintForeground)
                        {
                            isFirstPaintForeground = false;
                            if (m_paintForegroundContext) {
                                m_paintForegroundContext->update(*this);
                            }
                        }
                        QRectF rectF = QRectF{QPointF{x, y - fontMetricsF.ascent()},
                                              QSizeF{qreal(viewport()->width()), lineSpacing}};

                        const Qt::AlignmentFlag align = static_cast<const Qt::AlignmentFlag>(0);
                        int charIndex = 0;
                        const ForegroundFormatSet& formatSet = iter->second;
                        for (const LineFormatForeground& format : formatSet)
                        {
                            if (format.seg.charIndex > charIndex)
                            {
                                const QString& text = line.mid(charIndex, format.seg.charIndex - charIndex);
                                QRectF boundingRectF{};
                                painter.drawText(rectF, align, text, &boundingRectF);
                                rectF.moveLeft(boundingRectF.right());
                            }
                            const QString& text = line.mid(format.seg.charIndex, format.seg.length);
                            painter.save();
                            format.f->beforeDrawText(painter, m_paintForegroundContext.get());
                            QRectF boundingRectF{};
                            painter.drawText(rectF, align, text, &boundingRectF);
                            painter.restore();
                            rectF.moveLeft(boundingRectF.right());
                            charIndex = format.seg.charIndex + format.seg.length;
                        }
                        if (charIndex < line.length())
                        {
                            const QString& text = line.right(line.length() - charIndex);
                            painter.drawText(QPointF{rectF.x(), y}, text);
                        }
                    }
                }
                y += lineSpacing;
            }
            // Only adjust if we have a "full view": visibleLineCount == i
            if (visibleLineCount == visibleLineOffset && visibleLineCount > fullyVisibleLineCount) {
                --m_lastFullyVisibleLineIndex;
            }
            if (first != m_firstVisibleLineIndex || lastFull != m_lastFullyVisibleLineIndex || last != m_lastVisibleLineIndex)
            {
                emit signalVisibleLineIndicesChanged();
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
        Private::updateVisibleLineCounts(*this);
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
