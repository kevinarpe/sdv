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

enum class StopEventPropagation { Yes, No };

static const QString EMPTY_TEXT_LINE{};
static const QString SPACE_GRAPHEME{QLatin1Char{' '}};

// TODO: Mouse click will move cursor!
struct TextViewTextCursor::Private
{
    static void
    slotHorizontalScrollBarActionTriggered(TextViewTextCursor& self, QAbstractSlider::SliderAction action)
    {
        updateAfterMove(self);
    }

    static void
    slotVerticalScrollBarActionTriggered(TextViewTextCursor& self, QAbstractSlider::SliderAction action)
    {
        updateAfterMove(self);
    }

    static void
    update(TextViewTextCursor& self)
    {
        self.m_isUpdate = true;
        self.m_textView.viewport()->update(self.m_textView.textCursorRect());
    }

    static void
    updateAfterMove(TextViewTextCursor& self)
    {
        self.m_hasMoved = true;
        self.m_isUpdate = true;
        // Intentional: When moving the cursor, skip the next invisible state.
        // It is weird UX to have the cursor blinking when moving the cursor.
        show(self);
        self.m_textView.viewport()->update();
    }

    static void
    updateAfterSelectionChange(TextViewTextCursor& self)
    {
        self.m_textView.viewport()->update();
    }

    static void
    clearSelection(TextViewTextCursor& self)
    {
        if (self.m_selectionStartPos.isValid())
        {
            self.m_selectionStartPos.invalidate();
            updateAfterSelectionChange(self);
        }
    }

    static const QString&
    getLine(const TextViewTextCursor& self)
    {
        const TextViewPosition& pos = self.m_graphemeCursor->pos();
        const std::vector<QString>& textLineVec = self.m_docView->doc().lineVec();
        const QString& line = textLineVec.empty() ? EMPTY_TEXT_LINE : textLineVec[pos.lineIndex];
        return line;
    }

    static StopEventPropagation
    keyPressEvent(TextViewTextCursor& self, QKeyEvent* event)
    {
        const TextViewPosition& pos = self.m_graphemeCursor->pos();
        const QString& line = getLine(self);
        // Assume event will match.  If not, flip to No at very bottom.
        StopEventPropagation stopEventPropagation = StopEventPropagation::Yes;

        // Qt::Key::Key_Left
        if (event->matches(QKeySequence::StandardKey::MoveToPreviousChar))
        {
            if (0 == pos.graphemeIndex)
            {
                const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.lineIndex, -1);
                if (lineIndex != pos.lineIndex)
                {
                    self.m_graphemeCursor->setLineIndexThenEnd(lineIndex);
                    horizontalScrollToEnsureVisible(self);
                    verticalScrollToEnsureVisible(self);
                    updateAfterMove(self);
                }
            }
            else {  // if (self.m_pos.columnIndex > 0)
                self.m_graphemeCursor->left();
                horizontalScrollToEnsureVisible(self);
                updateAfterMove(self);
            }
            clearSelection(self);
        }
        // TODO: Selection!
        else if (event->matches(QKeySequence::StandardKey::SelectPreviousChar))
        {
            // LAST
        }
        // Qt::Key::Key_Right
        else if (event->matches(QKeySequence::StandardKey::MoveToNextChar))
        {
            if (self.m_graphemeCursor->isAtEnd())
            {
                const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.lineIndex, +1);
                if (lineIndex != pos.lineIndex)
                {
                    self.m_fontWidth.beforeGrapheme = 0;
                    scrollToMin(self.m_textView.horizontalScrollBar());
                    self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
                    verticalScrollToEnsureVisible(self);
                    updateAfterMove(self);
                }
            }
            else {  // if (self.m_pos.columnIndex < line.length())
                self.m_graphemeCursor->right();
                horizontalScrollToEnsureVisible(self);
                updateAfterMove(self);
            }
            clearSelection(self);
        }
        // Qt::Key::Key_Control + Qt::Key::Key_Left
        else if (event->matches(QKeySequence::StandardKey::MoveToPreviousWord))
        {
            int dummy = 1;
        }
        // Qt::Key::Key_Control + Qt::Key::Key_Right
        else if (event->matches(QKeySequence::StandardKey::MoveToNextWord))
        {
            const QRegularExpression rx{"\\b"};
            const QRegularExpressionMatch& match = rx.match(line, pos.charIndex);
            if (match.hasMatch()) {
                int dummy = 1;
            }
        }
        // Qt::Key::Key_Home
        else if (event->matches(QKeySequence::StandardKey::MoveToStartOfLine))
        {
            verticalScrollToEnsureVisible(self);
            if (0 != pos.graphemeIndex)
            {
                self.m_fontWidth.beforeGrapheme = 0;
                self.m_graphemeCursor->home();
                scrollToMin(self.m_textView.horizontalScrollBar());
                updateAfterMove(self);
            }
            clearSelection(self);
        }
        // Qt::Key::Key_End
        else if (event->matches(QKeySequence::StandardKey::MoveToEndOfLine))
        {
            verticalScrollToEnsureVisible(self);
            if (false == self.m_graphemeCursor->isAtEnd())
            {
                self.m_graphemeCursor->end();
                horizontalScrollToEnsureVisible(self);
                updateAfterMove(self);
            }
            clearSelection(self);
        }
        // Qt::Key::Key_Control + Qt::Key::Key_Home
        else if (event->matches(QKeySequence::StandardKey::MoveToStartOfDocument))
        {
            verticalScrollToEnsureVisible(self);
            const int lineIndex = self.m_docView->firstVisibleLineIndex();

            if (lineIndex != pos.lineIndex || 0 != pos.graphemeIndex)
            {
                self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
                self.m_fontWidth.beforeGrapheme = 0;
                scrollToMin(self.m_textView.horizontalScrollBar());
                scrollToMin(self.m_textView.verticalScrollBar());
                updateAfterMove(self);
            }
            clearSelection(self);
        }
        // Qt::Key::Key_Control + Qt::Key::Key_End
        else if (event->matches(QKeySequence::StandardKey::MoveToEndOfDocument))
        {
            verticalScrollToEnsureVisible(self);
            const int lineIndex = self.m_docView->lastVisibleLineIndex();

            if (lineIndex != pos.lineIndex || self.m_graphemeCursor->isAtEnd() == false)
            {
                self.m_graphemeCursor->setLineIndexThenEnd(lineIndex);
                horizontalScrollToEnsureVisible(self);
                scrollToMax(self.m_textView.verticalScrollBar());
                updateAfterMove(self);
            }
            clearSelection(self);
        }
        // Qt::Key::Key_Up
        else if (event->matches(QKeySequence::StandardKey::MoveToPreviousLine))
        {
            const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.lineIndex, -1);
            verticalScrollToEnsureVisible(self);
            if (lineIndex != pos.lineIndex)
            {
                verticalMove(self, lineIndex);
                verticalScrollToEnsureVisible(self);
            }
            clearSelection(self);
        }
        // Qt::Key::Key_Down
        else if (event->matches(QKeySequence::StandardKey::MoveToNextLine))
        {
            const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.lineIndex, +1);
            verticalScrollToEnsureVisible(self);
            if (lineIndex != pos.lineIndex)
            {
                verticalMove(self, lineIndex);
                verticalScrollToEnsureVisible(self);
            }
            clearSelection(self);
        }
        // Qt::Key::Key_PageUp
        else if (event->matches(QKeySequence::StandardKey::MoveToPreviousPage))
        {
            const int fullyVisibleLineCount = self.m_textView.verticalScrollBar()->pageStep();
            const int lineIndex =
                self.m_docView->nextVisibleLineIndex(pos.lineIndex, -1 * fullyVisibleLineCount);

            verticalScrollToEnsureVisible(self);

            if (lineIndex != pos.lineIndex)
            {
                scrollPage(self.m_textView.verticalScrollBar(), ScrollDirection::Up);
                verticalMove(self, lineIndex);
            }
            clearSelection(self);
        }
        // Qt::Key::Key_PageDown
        else if (event->matches(QKeySequence::StandardKey::MoveToNextPage))
        {
            const int fullyVisibleLineCount = self.m_textView.verticalScrollBar()->pageStep();
            const int lineIndex = self.m_docView->nextVisibleLineIndex(pos.lineIndex, fullyVisibleLineCount);
            verticalScrollToEnsureVisible(self);

            if (lineIndex != pos.lineIndex)
            {
                scrollPage(self.m_textView.verticalScrollBar(), ScrollDirection::Down);
                verticalMove(self, lineIndex);
            }
            clearSelection(self);
        }
        else {
            // See: bool QKeyEvent::matches(QKeySequence::StandardKey matchKey) const
            // "The keypad and group switch modifier should not make a difference"
            const uint searchkey = (event->modifiers() | event->key()) & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
            if (searchkey == (Qt::Modifier::CTRL | Qt::Key::Key_PageUp))
            {
                // TODO: Move to first visible line in viewport
            }
            else if (searchkey == (Qt::Modifier::CTRL | Qt::Key::Key_PageDown))
            {
                // TODO: Move to last (fully) visible line in viewport -- no scrolling!
            }
            // THIS IS TOUGH TO IMPL! :P
//            else if (searchkey == (Qt::Modifier::ALT | Qt::Key::Key_PageUp))
//            {
//                verticalScrollToEnsureVisible(self);
//                QScrollBar* const hbar = self.m_textView.horizontalScrollBar();
//                if (hbar->value() > hbar->minimum())
//                {
//                    const int v = std::max(hbar->minimum(), hbar->value() - hbar->pageStep());
//                    scrollToValue(hbar, v);
//                    updateAfterMove(self);
//                }
//                clearSelection(self);
//            }
//            else if (searchkey == (Qt::Modifier::ALT | Qt::Key::Key_PageDown))
//            {
//                verticalScrollToEnsureVisible(self);
//                QScrollBar* const hbar = self.m_textView.horizontalScrollBar();
//                if (hbar->value() < hbar->maximum())
//                {
//                    const int v = std::min(hbar->maximum(), hbar->value() + hbar->pageStep());
//                    scrollToValue(hbar, v);
//                    updateAfterMove(self);
//                }
//                clearSelection(self);
//            }
//            else
            {
                stopEventPropagation = StopEventPropagation::No;
            }
        }
        return stopEventPropagation;
    }

    static void
    verticalMove(TextViewTextCursor& self, const int lineIndex)
    {
        // Important: Copy, not const ref here, as setLineIndexThenHome() will update the const ref!
        const TextViewPosition origPos = self.m_graphemeCursor->pos();
        assert(lineIndex != origPos.lineIndex);
        self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
        const QString& line = getLine(self);
        if (0 == origPos.charIndex || line.isEmpty())
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
        updateAfterMove(self);
    }

    static void
    horizontalScrollToEnsureVisible(TextViewTextCursor& self)
    {
        const QString& line = getLine(self);
        const QFontMetricsF fontMetricsF{self.m_textView.font()};
        const TextViewPosition& pos = self.m_graphemeCursor->pos();
        self.m_fontWidth.beforeGrapheme = fontMetricsF.horizontalAdvance(line, pos.charIndex);
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
        const TextViewPosition& pos = self.m_graphemeCursor->pos();
        if (pos.lineIndex < self.m_textView.firstVisibleLineIndex())
        {
            scrollToLineIndex(self, pos.lineIndex);
        }
        else if (pos.lineIndex > self.m_textView.lastFullyVisibleLineIndex())
        {
            QScrollBar* const vbar = self.m_textView.verticalScrollBar();
            const int vbarValue = std::max(0, pos.lineIndex - (vbar->pageStep() - 1));
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

    /**
     * If blinking, restart timer and make visible.
     */
    static void
    show(TextViewTextCursor& self)
    {
        if (self.m_isBlinking)
        {
            startTimer(self);
            self.m_isVisible = true;
            update(self);
        }
    }

    static void
    focusOutEvent(TextViewTextCursor& self, QFocusEvent* event)
    {
        if (self.m_isBlinking)
        {
            self.m_timer.stop();
            self.m_isVisible = true;
            update(self);
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
            if (Qt::KeyboardModifier::ShiftModifier == QGuiApplication::keyboardModifiers()) {
                self.m_selectionStartPos = self.m_graphemeCursor->pos();
            }
            const StopEventPropagation x = mouseMoveEvent(self, event);
            assert(StopEventPropagation::Yes == x);
            return x;
        }
        return StopEventPropagation::No;
    }

    static StopEventPropagation
    mouseMoveEvent(TextViewTextCursor& self, QMouseEvent* event)
    {
        // event->button(): "Note that the returned value is always Qt::NoButton for mouse move events."
        const int lineIndex = self.m_textView.lineIndexForHeight(event->pos().y());
        const TextViewPosition& pos = self.m_graphemeCursor->pos();
        if (lineIndex != pos.lineIndex) {
            self.m_graphemeCursor->setLineIndexThenHome(lineIndex);
        }
        const QFontMetricsF fontMetricsF{self.m_textView.font()};
        const TextSegmentFontWidth fontWidth = self.m_graphemeCursor->horizontalMove(fontMetricsF, event->pos().x());
        self.m_fontWidth = fontWidth;
        horizontalScrollToEnsureVisible(self, fontWidth);
        verticalScrollToEnsureVisible(self);
        updateAfterMove(self);
        return StopEventPropagation::Yes;
    }
    // Intentional: There is no need to capture mouseReleaseEvent for LeftButton.
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
      m_selectionStartPos{TextViewPosition::invalid()},
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
}

// public
void
TextViewTextCursor::
reset()
{
    m_graphemeCursor->reset();
}

// public
const TextViewPosition&
TextViewTextCursor::
pos()
const
{
    return m_graphemeCursor->pos();
}

// public
const QString&
TextViewTextCursor::
grapheme()
const
{
    const QString& x = m_graphemeCursor->pos().grapheme;
    return x;
}

// public
TextViewSelection
TextViewTextCursor::
selection()
const
{
    const TextViewSelection x{
        .beginInclusive = std::min(m_selectionStartPos, m_graphemeCursor->pos()),
        .endExclusive = std::max(m_selectionStartPos, m_graphemeCursor->pos())
    };
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
        Private::update(*this);
    }
}

}  // namespace SDV
