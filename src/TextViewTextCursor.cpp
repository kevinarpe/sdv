//
// Created by kca on 10/6/2020.
//

#include "TextViewTextCursor.h"
#include <QApplication>
#include <QKeyEvent>
#include <QDebug>
#include <QScrollBar>
#include <QRegularExpression>
#include "TextView.h"
#include "TextViewDocument.h"
#include "TextViewDocumentView.h"
#include "TextViewGraphemeCursor.h"

namespace SDV {

// public static
const QChar TextViewTextCursor::SPACE_CHAR{QLatin1Char{' '}};
// public static
const QString TextViewTextCursor::SPACE_GRAPHEME{SPACE_CHAR};

enum class StopEventPropagation { Yes, No };
static const QString EMPTY_TEXT_LINE{};

struct TextViewTextCursor::Private
{
    static void
    slotHorizontalScrollBarActionTriggered(TextViewTextCursor& self,
                                           const QAbstractSlider::SliderAction action)
    {
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        // Intentional: Treat this as a move.  Horizontal scrolling shifts the on-screen position of text cursor.
        updateAfterMove(self, pos);
    }

    static void
    slotVerticalScrollBarActionTriggered(TextViewTextCursor& self,
                                         const QAbstractSlider::SliderAction action)
    {
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        // Intentional: Treat this as a move.  Vertical scrolling shifts the on-screen position of text cursor.
        updateAfterMove(self, pos);
    }

    static void
    slotVisibleLineIndicesChanged(TextViewTextCursor& self)
    {
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        if (pos.pos.lineIndex < self.m_textView.firstVisibleLineIndex()
            || pos.pos.lineIndex > self.m_textView.lastVisibleLineIndex())
        {
            self.m_timer.stop();
        }
    }

    static const QString&
    getLine(const TextViewTextCursor& self)
    {
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        const std::vector<QString>& textLineVec = self.m_docView->doc().lineVec();
        const QString& line = textLineVec.empty() ? EMPTY_TEXT_LINE : textLineVec[pos.pos.lineIndex];
        return line;
    }

    static StopEventPropagation
    keyPressEvent(TextViewTextCursor& self, QKeyEvent* event)
    {
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        // Assume event will match.  If not, flip to No at very bottom.
        StopEventPropagation stopEventPropagation = StopEventPropagation::Yes;

        // Ref: https://doc.qt.io/qt-5/qkeysequence.html#standard-shortcuts
        // Qt::Key::Key_Left
        if (event->matches(QKeySequence::StandardKey::MoveToPreviousChar))
        {
            move(self, &moveLeft);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Left
        else if (event->matches(QKeySequence::StandardKey::SelectPreviousChar))
        {
            moveSelect(self, &moveLeft);
        }
        // Qt::Key::Key_Right
        else if (event->matches(QKeySequence::StandardKey::MoveToNextChar))
        {
            move(self, &moveRight);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Right
        else if (event->matches(QKeySequence::StandardKey::SelectNextChar))
        {
            moveSelect(self, &moveRight);
        }
        // Qt::Key::Key_Control + Qt::Key::Key_Left
        else if (event->matches(QKeySequence::StandardKey::MoveToPreviousWord))
        {
            // TODO
            int dummy = 1;
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Control + Qt::Key::Key_Left
        else if (event->matches(QKeySequence::StandardKey::SelectPreviousWord))
        {
            // TODO
            int dummy = 1;
        }
        // Qt::Key::Key_Control + Qt::Key::Key_Right
        else if (event->matches(QKeySequence::StandardKey::MoveToNextWord))
        {
            // Next/Prev word seems "broken" in IntelliJ.  Think more deeply about this first!
            if (false)
            {
                const QRegularExpression rx{"\\b"};
                verticalScrollToEnsureVisible(self);
                const TextViewGraphemePosition origPos = self.m_graphemeCursor->pos();
                const std::vector<QString>& textLineVec = self.m_docView->doc().lineVec();
                const auto iter0 = self.m_docView->find(origPos.pos.lineIndex);
                const std::vector<int>& visibleLineIndexVec = self.m_docView->visibleLineIndexVec();

                for (auto iter = iter0 ; visibleLineIndexVec.end() != iter ; ++iter)
                {
                    const int lineIndex = *iter;
                    const QString& line = textLineVec[lineIndex];
                    const int charIndex = (iter0 == iter) ? origPos.pos.charIndex : 0;
                    const QRegularExpressionMatch& match = rx.match(line, 1 + charIndex);
                    if (match.hasMatch())
                    {
                        if (lineIndex != origPos.pos.lineIndex)
                        {
                            self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
                        }
                        const int charIndex = match.capturedStart();
                        self.m_graphemeCursor->setCharIndex(charIndex);
                        break;
                    }
                }
                const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
                if (false == origPos.pos.isEqual(pos.pos)) {
//                    updateAfterMove(self, lineChanged);
                }
            }
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Control + Qt::Key::Key_Right
        else if (event->matches(QKeySequence::StandardKey::SelectNextWord))
        {
            // TODO
            int dummy = 1;
        }
        // Qt::Key::Key_Home
        else if (event->matches(QKeySequence::StandardKey::MoveToStartOfLine))
        {
            move(self, &moveHome);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Home
        else if (event->matches(QKeySequence::StandardKey::SelectStartOfLine))
        {
            moveSelect(self, &moveHome);
        }
        // Qt::Key::Key_End
        else if (event->matches(QKeySequence::StandardKey::MoveToEndOfLine))
        {
            move(self, &moveEnd);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_End
        else if (event->matches(QKeySequence::StandardKey::SelectEndOfLine))
        {
            moveSelect(self, &moveEnd);
        }
        // Qt::Key::Key_Control + Qt::Key::Key_Home
        else if (event->matches(QKeySequence::StandardKey::MoveToStartOfDocument))
        {
            move(self, &moveDocHome);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Control + Qt::Key::Key_Home
        else if (event->matches(QKeySequence::StandardKey::SelectStartOfDocument))
        {
            moveSelect(self, &moveDocHome);
        }
        // Qt::Key::Key_Control + Qt::Key::Key_End
        else if (event->matches(QKeySequence::StandardKey::MoveToEndOfDocument))
        {
            move(self, &moveDocEnd);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Control + Qt::Key::Key_End
        else if (event->matches(QKeySequence::StandardKey::SelectEndOfDocument))
        {
            moveSelect(self, &moveDocEnd);
        }
        // Qt::Key::Key_Up
        else if (event->matches(QKeySequence::StandardKey::MoveToPreviousLine))
        {
            move(self, &moveUp);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Up
        else if (event->matches(QKeySequence::StandardKey::SelectPreviousLine))
        {
            moveSelect(self, &moveUp);
        }
        // Qt::Key::Key_Down
        else if (event->matches(QKeySequence::StandardKey::MoveToNextLine))
        {
            move(self, &moveDown);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Down
        else if (event->matches(QKeySequence::StandardKey::SelectNextLine))
        {
            moveSelect(self, &moveDown);
        }
        // Qt::Key::Key_PageUp
        else if (event->matches(QKeySequence::StandardKey::MoveToPreviousPage))
        {
            move(self, &movePageUp);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_PageUp
        else if (event->matches(QKeySequence::StandardKey::SelectPreviousPage))
        {
            moveSelect(self, &movePageUp);
        }
        // Qt::Key::Key_PageDown
        else if (event->matches(QKeySequence::StandardKey::MoveToNextPage))
        {
            move(self, &movePageDown);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_PageDown
        else if (event->matches(QKeySequence::StandardKey::SelectNextPage))
        {
            moveSelect(self, &movePageDown);
        }
        // Qt::Key::Key_Control + Qt::Key::Key_A
        else if (event->matches(QKeySequence::StandardKey::SelectAll))
        {
            selectAll(self);
        }
        // Qt::Key::Key_Shift + Qt::Key::Key_Control + Qt::Key::Key_A
        else if (event->matches(QKeySequence::StandardKey::Deselect))
        {
            deselect(self);
        }
        else {
            // See: bool QKeyEvent::matches(QKeySequence::StandardKey matchKey) const
            // "The keypad and group switch modifier should not make a difference"
            const uint searchkey = (event->modifiers() | event->key()) & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
            if (searchkey == (Qt::Modifier::CTRL | Qt::Key::Key_PageUp))
            {
                move(self, &moveViewTop);
            }
            else if (searchkey == (Qt::Modifier::SHIFT | Qt::Modifier::CTRL | Qt::Key::Key_PageUp))
            {
                moveSelect(self, &moveViewTop);
            }
            else if (searchkey == (Qt::Modifier::CTRL | Qt::Key::Key_PageDown))
            {
                move(self, &moveViewBottom);
            }
            else if (searchkey == (Qt::Modifier::SHIFT | Qt::Modifier::CTRL | Qt::Key::Key_PageDown))
            {
                moveSelect(self, &moveViewBottom);
            }
            // THIS IS TOUGH TO IMPL! :P
            else if (searchkey == (Qt::Modifier::ALT | Qt::Key::Key_PageUp))
            {
                // TODO
                int dummy = 1;
//                verticalScrollToEnsureVisible(self);
//                QScrollBar* const hbar = self.m_textView.horizontalScrollBar();
//                if (hbar->value() > hbar->minimum())
//                {
//                    const int v = std::max(hbar->minimum(), hbar->value() - hbar->pageStep());
//                    scrollToValue(hbar, v);
//                    updateAfterMove(self);
//                }
//                clearSelection(self);
            }
            else if (searchkey == (Qt::Modifier::ALT | Qt::Key::Key_PageDown))
            {
                // TODO
                int dummy = 1;
//                verticalScrollToEnsureVisible(self);
//                QScrollBar* const hbar = self.m_textView.horizontalScrollBar();
//                if (hbar->value() < hbar->maximum())
//                {
//                    const int v = std::min(hbar->maximum(), hbar->value() + hbar->pageStep());
//                    scrollToValue(hbar, v);
//                    updateAfterMove(self);
//                }
//                clearSelection(self);
            }
            else
            {
                stopEventPropagation = StopEventPropagation::No;
            }
        }
        return stopEventPropagation;
    }

    static void
    updateCursorOnly(TextViewTextCursor& self)
    {
        self.m_isUpdate = true;
        self.m_textView.viewport()->update(self.m_textView.textCursorRect());
    }

    static void
    update(TextViewTextCursor& self)
    {
        self.m_isUpdate = true;
        self.m_textView.viewport()->update();
    }

    static void
    updateAfterMove(TextViewTextCursor& self, const TextViewGraphemePosition& origPos)
    {
        self.m_hasMoved = true;
        self.m_isUpdate = true;
        // Intentional: When moving the cursor, skip the next invisible state.
        // It is weird UX to have the cursor blinking when moving the cursor.
        show(self);
        self.m_textView.viewport()->update();
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        if (false == pos.pos.isEqual(origPos.pos))
        {
            // Only enable if we need later.
//            emit self.signalPositionChanged();
        }
        // Intentional: Not else-if
        if (pos.pos.lineIndex != origPos.pos.lineIndex)
        {
            emit self.signalLineChange(pos.pos.lineIndex);
        }
    }

    /* Ex: &moveLeft */
    using MoveFunc = bool (*)(TextViewTextCursor& self);

    static void
    move(TextViewTextCursor& self, MoveFunc moveFunc)
    {
        // Intentional: Copy not reference.  Why?  moveFunc may change position.
        const TextViewGraphemePosition origPos = self.m_graphemeCursor->pos();
        if (moveFunc(self))
        {
            updateAfterMove(self, origPos);
            clearSelection(self);
        }
    }

    static void
    clearSelection(TextViewTextCursor& self)
    {
        if (self.m_selection.isValid())
        {
            self.m_selection.invalidate();
            update(self);
        }
    }

    static void
    moveSelect(TextViewTextCursor& self, MoveFunc moveFunc)
    {
        // Intentional: Copy not reference.  Why?  moveFunc may change position.
        const TextViewGraphemePosition origPos = self.m_graphemeCursor->pos();
        if (moveFunc(self))
        {
            const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
            if (self.m_selection.begin.isValid())
            {
                // Forward or backward selection cross-over
                if (pos.pos.isEqual(self.m_selection.begin))
                {
                    self.m_selection.begin.invalidate();
                }
            }
            else {
                self.m_selection.begin = origPos.pos;
            }
            self.m_selection.end = pos.pos;
            updateAfterMove(self, origPos);
        }
    }

    static bool
    moveLeft(TextViewTextCursor& self)
    {
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        if (0 == pos.graphemeIndex)
        {
            const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.pos.lineIndex, -1);
            if (lineIndex != pos.pos.lineIndex)
            {
                self.m_graphemeCursor->setLineIndexThenEnd(lineIndex);
                horizontalScrollToEnsureVisible(self);
                verticalScrollToEnsureVisible(self);
                return true;
            }
        }
        else {  // if (self.m_pos.columnIndex > 0)
            self.m_graphemeCursor->left();
            horizontalScrollToEnsureVisible(self);
            return true;
        }
        return false;
    }

    static bool
    moveRight(TextViewTextCursor& self)
    {
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        if (self.m_graphemeCursor->isAtEnd())
        {
            const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.pos.lineIndex, +1);
            if (lineIndex != pos.pos.lineIndex)
            {
                self.m_fontWidth.beforeGrapheme = 0;
                scrollToMin(self.m_textView.horizontalScrollBar());
                self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
                verticalScrollToEnsureVisible(self);
                return true;
            }
        }
        else {  // if (self.m_pos.columnIndex < line.length())
            self.m_graphemeCursor->right();
            horizontalScrollToEnsureVisible(self);
            return true;
        }
        return false;
    }

    static bool
    moveHome(TextViewTextCursor& self)
    {
        verticalScrollToEnsureVisible(self);
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();

        if (0 != pos.graphemeIndex)
        {
            self.m_fontWidth.beforeGrapheme = 0;
            self.m_graphemeCursor->home();
            scrollToMin(self.m_textView.horizontalScrollBar());
            return true;
        }
        return false;
    }

    static bool
    moveEnd(TextViewTextCursor& self)
    {
        verticalScrollToEnsureVisible(self);
        if (false == self.m_graphemeCursor->isAtEnd())
        {
            self.m_graphemeCursor->end();
            horizontalScrollToEnsureVisible(self);
            return true;
        }
        return false;
    }

    static bool
    moveDocHome(TextViewTextCursor& self)
    {
        verticalScrollToEnsureVisible(self);
        const int lineIndex = self.m_docView->firstVisibleLineIndex();
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();

        if (lineIndex != pos.pos.lineIndex || 0 != pos.graphemeIndex)
        {
            self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
            self.m_fontWidth.beforeGrapheme = 0;
            scrollToMin(self.m_textView.horizontalScrollBar());
            scrollToMin(self.m_textView.verticalScrollBar());
            return true;
        }
        return false;
    }

    static bool
    moveDocEnd(TextViewTextCursor& self)
    {
        verticalScrollToEnsureVisible(self);
        const int lineIndex = self.m_docView->lastVisibleLineIndex();
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();

        if (lineIndex != pos.pos.lineIndex || self.m_graphemeCursor->isAtEnd() == false)
        {
            self.m_graphemeCursor->setLineIndexThenEnd(lineIndex);
            horizontalScrollToEnsureVisible(self);
            scrollToMax(self.m_textView.verticalScrollBar());
            return true;
        }
        return false;
    }

    static bool
    moveUp(TextViewTextCursor& self)
    {
        verticalScrollToEnsureVisible(self);
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.pos.lineIndex, -1);

        if (lineIndex != pos.pos.lineIndex)
        {
            verticalMove(self, lineIndex);
            verticalScrollToEnsureVisible(self);
            return true;
        }
        return false;
    }

    static bool
    moveDown(TextViewTextCursor& self)
    {
        verticalScrollToEnsureVisible(self);
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.pos.lineIndex, +1);

        if (lineIndex != pos.pos.lineIndex)
        {
            verticalMove(self, lineIndex);
            verticalScrollToEnsureVisible(self);
            return true;
        }
        return false;
    }

    static bool
    movePageUp(TextViewTextCursor& self)
    {
        const int fullyVisibleLineCount = self.m_textView.verticalScrollBar()->pageStep();
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.pos.lineIndex, -1 * fullyVisibleLineCount);
        verticalScrollToEnsureVisible(self);

        if (lineIndex != pos.pos.lineIndex)
        {
            scrollPage(self.m_textView.verticalScrollBar(), ScrollDirection::Up);
            verticalMove(self, lineIndex);
            return true;
        }
        return false;
    }

    static bool
    movePageDown(TextViewTextCursor& self)
    {
        const int fullyVisibleLineCount = self.m_textView.verticalScrollBar()->pageStep();
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.pos.lineIndex, fullyVisibleLineCount);
        verticalScrollToEnsureVisible(self);

        if (lineIndex != pos.pos.lineIndex)
        {
            scrollPage(self.m_textView.verticalScrollBar(), ScrollDirection::Down);
            verticalMove(self, lineIndex);
            return true;
        }
        return false;
    }

    static bool
    moveViewTop(TextViewTextCursor& self)
    {
        verticalScrollToEnsureVisible(self);
        const int lineIndex = self.m_textView.firstVisibleLineIndex();
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();

        if (lineIndex != pos.pos.lineIndex) {
            verticalMove(self, lineIndex);
            return true;
        }
        return false;
    }

    static bool
    moveViewBottom(TextViewTextCursor& self)
    {
        verticalScrollToEnsureVisible(self);
        const int lineIndex = self.m_textView.lastFullyVisibleLineIndex();
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();

        if (lineIndex != pos.pos.lineIndex) {
            verticalMove(self, lineIndex);
            return true;
        }
        return false;
    }

    static void
    selectAll(TextViewTextCursor& self)
    {
        const TextViewSelection origSelection = self.m_selection;
        const std::vector<QString>& lineVec = self.m_docView->doc().lineVec();
        if (lineVec.empty())
        {
            self.m_selection.begin.invalidate();
            self.m_selection.end.invalidate();
        }
        else {
            // Begin
            {
                const int firstVisibleLineIndex = self.m_docView->firstVisibleLineIndex();
                const QString& firstVisibleLine = lineVec[firstVisibleLineIndex];
                if (firstVisibleLine.isEmpty())
                {
                    self.m_selection.begin = TextViewPosition{.lineIndex = firstVisibleLineIndex, .charIndex = 0};
                }
                else {
                    QTextBoundaryFinder f{QTextBoundaryFinder::BoundaryType::Grapheme, firstVisibleLine};
                    const int b = f.toNextBoundary();
                    assert(-1 != b);
                    const QString& grapheme = firstVisibleLine.left(b);
                    self.m_selection.begin = TextViewPosition{.lineIndex = firstVisibleLineIndex, .charIndex = 0};
                }
            }
            // End
            {
                const int lastVisibleLineIndex = self.m_docView->lastVisibleLineIndex();
                const QString& lastVisibleLine = lineVec[lastVisibleLineIndex];
                if (lastVisibleLine.isEmpty())
                {
                    self.m_selection.begin = TextViewPosition{.lineIndex = lastVisibleLineIndex, .charIndex = 0};
                }
                else {
                    QTextBoundaryFinder f{QTextBoundaryFinder::BoundaryType::Grapheme, lastVisibleLine};
                    int graphemeIndex = 0;
                    while (-1 != f.toNextBoundary()) {
                        ++graphemeIndex;
                    }
                    self.m_selection.begin =
                        TextViewPosition{.lineIndex = lastVisibleLineIndex, .charIndex = lastVisibleLine.length()};
                }
            }
        }
        if (false == origSelection.isEqual(self.m_selection))
        {
            update(self);
        }
    }

    static void
    deselect(TextViewTextCursor& self)
    {
        clearSelection(self);
    }

    static void
    verticalMove(TextViewTextCursor& self, const int lineIndex)
    {
        // Important: Copy, not const ref here, as setLineIndexThenHome() will update the const ref!
        const TextViewGraphemePosition origPos = self.m_graphemeCursor->pos();
        assert(lineIndex != origPos.pos.lineIndex);
        self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
        const QString& line = getLine(self);
        if (0 == origPos.pos.charIndex || line.isEmpty())
        {
            scrollToMin(self.m_textView.horizontalScrollBar());
        }
        else {
            const QFontMetricsF fontMetricsF{self.m_textView.font()};
            const qreal fontWidthF = self.m_fontWidth.beforeGrapheme + (0.5 * self.m_fontWidth.grapheme);
            const TextSegmentFontWidth fontWidth = self.m_graphemeCursor->horizontalMove(fontMetricsF, fontWidthF);
            horizontalScrollToEnsureVisible(self, fontWidth);
            // Intentional: Do not call verticalScrollToEnsureVisible() here.  It will break PageUp/Down. :)
        }
    }

    static void
    horizontalScrollToEnsureVisible(TextViewTextCursor& self)
    {
        const QString& line = getLine(self);
        const QFontMetricsF fontMetricsF{self.m_textView.font()};
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        self.m_fontWidth.beforeGrapheme = fontMetricsF.horizontalAdvance(line, pos.pos.charIndex);
        self.m_fontWidth.grapheme = fontMetricsF.horizontalAdvance(pos.grapheme);
        horizontalScrollToEnsureVisible(self, self.m_fontWidth);
    }

    static void
    horizontalScrollToEnsureVisible(TextViewTextCursor& self, const TextSegmentFontWidth& fontWidth)
    {
        QScrollBar* const hbar = self.m_textView.horizontalScrollBar();
        if (fontWidth.beforeGrapheme < hbar->value())
        {
            // scroll left
            scrollToValue(hbar, fontWidth.beforeGrapheme);
        }
        else {
            const int width = qRound(fontWidth.beforeGrapheme + fontWidth.grapheme);
            const int viewportWidth = self.m_textView.viewport()->width();
            if (width > hbar->value() + viewportWidth)
            {
                // scroll right
                const int hbarValue = width - viewportWidth;
                scrollToValue(hbar, hbarValue);
            }
        }
    }

    static void
    verticalScrollToEnsureVisible(TextViewTextCursor& self)
    {
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        if (pos.pos.lineIndex < self.m_textView.firstVisibleLineIndex())
        {
            scrollToLineIndex(self, pos.pos.lineIndex);
        }
        else if (pos.pos.lineIndex > self.m_textView.lastFullyVisibleLineIndex())
        {
            QScrollBar* const vbar = self.m_textView.verticalScrollBar();
            const int vbarValue = std::max(0, pos.pos.lineIndex - (vbar->pageStep() - 1));
            scrollToLineIndex(self, vbarValue);
        }
    }

    static void
    scrollToMin(QScrollBar* const bar)
    {
        const int value = bar->value();
        const int min = bar->minimum();
        if (value > min) {
            scrollToValue(bar, min);
        }
    }

    static void
    scrollToMax(QScrollBar* const bar)
    {
        const int value = bar->value();
        const int max = bar->maximum();
        if (value < max) {
            scrollToValue(bar, max);
        }
    }

    enum ScrollDirection { Up = -1, Left = Up, Down = +1, Right = Down };

    static void
    scrollPage(QScrollBar* const bar, const ScrollDirection d)
    {
        const int prevValue = bar->value();
        const int value = prevValue + (d * bar->pageStep());
        scrollToValue(bar, value);
    }

    static void
    scrollToValue(QScrollBar* const bar, const int value)
    {
        bar->blockSignals(true);
        bar->setValue(value);
        bar->blockSignals(false);
    }

    static void
    scrollToLineIndex(const TextViewTextCursor& self, const int lineIndex)
    {
        const int normalisedLineIndex = self.m_docView->findNormalisedLineIndex(lineIndex);
        scrollToValue(self.m_textView.verticalScrollBar(), normalisedLineIndex);
    }

    static void
    focusInEvent(TextViewTextCursor& self, QFocusEvent* event)
    {
        show(self);
    }

    /** If blinking, restart timer and make visible. */
    static void
    show(TextViewTextCursor& self)
    {
        if (self.m_isBlinking)
        {
            startTimer(self);
            self.m_isVisible = true;
            updateCursorOnly(self);
        }
    }

    static void
    focusOutEvent(TextViewTextCursor& self, QFocusEvent* event)
    {
        if (self.m_isBlinking)
        {
            self.m_timer.stop();
            self.m_isVisible = true;
            updateCursorOnly(self);
        }
    }

    static void
    startTimer(TextViewTextCursor& self)
    {
        self.m_timer.stop();
        self.m_timer.start(self.m_blinkMillis, &self);
    }

    static void
    tryStartTimer(TextViewTextCursor& self)
    {
        if (self.m_textView.hasFocus()) {
            startTimer(self);
        }
    }

    // Do not forget Shift+LeftMouseButton will do an "instant" selection!
    static StopEventPropagation
    mousePressEvent(TextViewTextCursor& self, QMouseEvent* event)
    {
        if (Qt::MouseButton::LeftButton == event->button())
        {
            const bool isShift = (Qt::KeyboardModifier::ShiftModifier == QGuiApplication::keyboardModifiers());
            if (isShift)
            {
                const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
                self.m_selection.begin = pos.pos;
            }
            else {
                clearSelection(self);
            }
            mouseButtonPressOrMoveEvent(self, event);
            return StopEventPropagation::Yes;
        }
        return StopEventPropagation::No;
    }

    static StopEventPropagation
    mouseMoveEvent(TextViewTextCursor& self, QMouseEvent* event)
    {
        mouseButtonPressOrMoveEvent(self, event);
        return StopEventPropagation::Yes;
    }
    // Intentional: There is no need to capture mouseReleaseEvent for LeftButton.

    static void
    mouseButtonPressOrMoveEvent(TextViewTextCursor& self, QMouseEvent* event)
    {
        // event->button(): "Note that the returned value is always Qt::NoButton for mouse move events."
        const TextViewGraphemePosition origPos = self.m_graphemeCursor->pos();
        const int lineIndex = self.m_textView.lineIndexForHeight(event->pos().y());
        if (lineIndex != origPos.pos.lineIndex)
        {
            self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
        }
        const QFontMetricsF fontMetricsF{self.m_textView.font()};
        const TextSegmentFontWidth fontWidth = self.m_graphemeCursor->horizontalMove(fontMetricsF, event->pos().x());
        const TextViewGraphemePosition& pos = self.m_graphemeCursor->pos();
        if (origPos.pos.isEqual(pos.pos)) {
            return;
        }
        self.m_fontWidth = fontWidth;
        if (QEvent::Type::MouseButtonPress == event->type())
        {
            const bool isShift = (Qt::KeyboardModifier::ShiftModifier == QGuiApplication::keyboardModifiers());
            if (false == isShift)
            {
                self.m_selection.begin = pos.pos;
                // Explicit: A single left mouse button click without shift key should *not* create a valid selection!
                self.m_selection.end.invalidate();
            }
        }
        else if (QEvent::Type::MouseMove == event->type())
        {
            if (pos.pos.isEqual(self.m_selection.begin)) {
                // Mouse move cross-over original selection begin.
                self.m_selection.end.invalidate();
            }
            else {
                self.m_selection.end = pos.pos;
            }
        }
        else {
            assert(false && event->type());
        }
        horizontalScrollToEnsureVisible(self, fontWidth);
        verticalScrollToEnsureVisible(self);
        updateAfterMove(self, origPos);
    }
};

// public
TextViewTextCursor::
TextViewTextCursor(TextView& textView, const std::shared_ptr<TextViewDocumentView>& docView)
    : Base{&textView},
      m_textView{textView},
      m_docView{docView},
      // Intentional: Below, immediately call slotSetBlinking(true)
      m_isBlinking{false},
      m_blinkMillis{qApp->cursorFlashTime()},
      m_isVisible{true},
//      m_graphemeCursor{std::make_unique<TextViewGraphemeCursor>(docView)},
      m_graphemeCursor{std::make_shared<TextViewGraphemeCursor>(docView)},
      m_fontWidth{TextSegmentFontWidth{.beforeGrapheme = 0, .grapheme = 0}},
      m_selection{TextViewSelection::invalid()},
      m_isUpdate{false},
      m_hasMoved{false}
{
    textView.installEventFilter(this);
    slotSetBlinking(true);

    QObject::connect(textView.horizontalScrollBar(), &QScrollBar::actionTriggered,
        [this](int action) {
            Private::slotHorizontalScrollBarActionTriggered(*this, static_cast<QAbstractSlider::SliderAction>(action));
        });
    QObject::connect(textView.verticalScrollBar(), &QScrollBar::actionTriggered,
        [this](int action) {
            Private::slotVerticalScrollBarActionTriggered(*this, static_cast<QAbstractSlider::SliderAction>(action));
        });
    QObject::connect(&textView, &TextView::signalVisibleLineIndicesChanged,
        [this]() { Private::slotVisibleLineIndicesChanged(*this); });
}

// public
void
TextViewTextCursor::
reset()
{
    m_graphemeCursor->reset();
}

// public
const TextViewGraphemePosition&
TextViewTextCursor::
position()
const
{
    const TextViewGraphemePosition& x = m_graphemeCursor->pos();
    return x;
}

// public
void
TextViewTextCursor::
afterPaintEvent()
{
    m_isUpdate = false;
    m_hasMoved = false;
}

/**
 * @return true to stop event propagation
 */
// public
bool
TextViewTextCursor::
eventFilter(QObject* watched, QEvent* event)  // override
{
    // Intentional: Base impl always returns false.
//    return QObject::eventFilter(watched, event);

    const QEvent::Type type = event->type();
    switch (type)
    {
        case QEvent::Type::KeyPress: {
            const StopEventPropagation x = Private::keyPressEvent(*this, static_cast<QKeyEvent*>(event));
            return (StopEventPropagation::Yes == x);
        }
        case QEvent::Type::FocusIn: {
            Private::focusInEvent(*this, static_cast<QFocusEvent*>(event));
            break;
        }
        case QEvent::Type::FocusOut: {
            Private::focusOutEvent(*this, static_cast<QFocusEvent*>(event));
            break;
        }
        case QEvent::Type::MouseButtonPress: {
            const StopEventPropagation x = Private::mousePressEvent(*this, static_cast<QMouseEvent*>(event));
            return (StopEventPropagation::Yes == x);
        }
        case QEvent::Type::MouseMove: {
            const StopEventPropagation x = Private::mouseMoveEvent(*this, static_cast<QMouseEvent*>(event));
            return (StopEventPropagation::Yes == x);
        }
        default: {
            // @DebugBreakpoint
            int dummy = 1;
        }
    }
    // Intentional: Never block processing by intended target.
    return false;
}

// public slot
void
TextViewTextCursor::
slotSetBlinking(const bool isBlinking)
{
    if (isBlinking != m_isBlinking)
    {
        m_isBlinking = isBlinking;
        if (isBlinking) {
            Private::tryStartTimer(*this);
        }
        else {
            m_timer.stop();
        }
    }
}

// public slot
void
TextViewTextCursor::
slotSetBlinkMillis(const int millis)
{
    if (millis != m_blinkMillis)
    {
        m_blinkMillis = millis;
        if (m_timer.isActive()) {
            Private::tryStartTimer(*this);
        }
    }
}

// protected
void
TextViewTextCursor::
timerEvent(QTimerEvent* event)  // override
{
    // Intentional: Base impl is empty.
//    Base::timerEvent(event);

    if (event->timerId() == m_timer.timerId()) {
        m_isVisible = ! m_isVisible;
        Private::updateCursorOnly(*this);
    }
}

}  // namespace SDV
