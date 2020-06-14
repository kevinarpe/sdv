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

namespace SDV {

enum class StopEventPropagation { Yes, No };

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

    static const QString&
    getLine(const TextViewTextCursor& self, const int lineIndex)
    {
        const std::vector<QString>& textLineVec = self.m_docView.doc().lineVec();
        const QString& line = textLineVec.empty() ? QString{} : textLineVec[lineIndex];
        return line;
    }

    static StopEventPropagation
    keyPressEvent(TextViewTextCursor& self, QKeyEvent* event)
    {
        const std::vector<QString>& textLineVec = self.m_docView.doc().lineVec();
        const QString& line = textLineVec.empty() ? QString{} : textLineVec[self.m_lineIndex];
        // Assume event will match.  If not, flip to No at very bottom.
        StopEventPropagation stopEventPropagation = StopEventPropagation::Yes;

        // Qt::Key::Key_Left
        if (event->matches(QKeySequence::StandardKey::MoveToPreviousChar))
        {
            if (0 == self.m_columnIndex)
            {
                const int lineIndex = self.m_docView.nextVisibleLineIndex(self.m_lineIndex, -1);
                if (lineIndex != self.m_lineIndex)
                {
                    const QString& prevLine = textLineVec[lineIndex];
                    self.m_columnIndex = prevLine.length();
                    horizontalScrollToEnsureVisible(self, prevLine, UpdateWidths::Yes);
                    self.m_lineIndex = lineIndex;
                    verticalScrollToEnsureVisible(self);
                    self.m_lineIndex = lineIndex;
                }
            }
            else {  // if (self.m_columnIndex > 0)
                --self.m_columnIndex;
                horizontalScrollToEnsureVisible(self, line, UpdateWidths::Yes);
                updateAfterMove(self);
            }
        }
        // TODO: Selection!
        else if (event->matches(QKeySequence::StandardKey::SelectPreviousChar))
        {
            // LAST
        }
        // Qt::Key::Key_Right
        else if (event->matches(QKeySequence::StandardKey::MoveToNextChar))
        {
            if (line.length() == self.m_columnIndex)
            {
                const int lineIndex = self.m_docView.nextVisibleLineIndex(self.m_lineIndex, +1);
                if (lineIndex != self.m_lineIndex)
                {
                    self.m_textBeforeWidth = self.m_columnIndex = 0;
                    scrollToMin(self.m_textView.horizontalScrollBar());
                    self.m_lineIndex = lineIndex;
                    verticalScrollToEnsureVisible(self);
                    updateAfterMove(self);
                }
            }
            else {  // if (self.m_columnIndex < line.length())
                ++self.m_columnIndex;
                horizontalScrollToEnsureVisible(self, line, UpdateWidths::Yes);
                updateAfterMove(self);
            }
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
            const QRegularExpressionMatch& match = rx.match(line, self.m_columnIndex);
            if (match.hasMatch()) {
                int dummy = 1;
            }
        }
        // Qt::Key::Key_Home
        else if (event->matches(QKeySequence::StandardKey::MoveToStartOfLine))
        {
            verticalScrollToEnsureVisible(self);
            if (0 != self.m_columnIndex)
            {
                self.m_textBeforeWidth = self.m_columnIndex = 0;
                scrollToMin(self.m_textView.horizontalScrollBar());
                updateAfterMove(self);
            }
        }
        // Qt::Key::Key_End
        else if (event->matches(QKeySequence::StandardKey::MoveToEndOfLine))
        {
            verticalScrollToEnsureVisible(self);
            if (line.length() != self.m_columnIndex)
            {
                self.m_columnIndex = line.length();
                horizontalScrollToEnsureVisible(self, line, UpdateWidths::Yes);
                updateAfterMove(self);
            }
        }
        // Qt::Key::Key_Control + Qt::Key::Key_Home
        else if (event->matches(QKeySequence::StandardKey::MoveToStartOfDocument))
        {
            verticalScrollToEnsureVisible(self);
            const int lineIndex = self.m_docView.firstVisibleLineIndex();

            if (lineIndex != self.m_lineIndex || 0 != self.m_columnIndex)
            {
                self.m_lineIndex = lineIndex;
                self.m_textBeforeWidth = self.m_columnIndex = 0;
                scrollToMin(self.m_textView.horizontalScrollBar());
                scrollToMin(self.m_textView.verticalScrollBar());
                updateAfterMove(self);
            }
        }
        // Qt::Key::Key_Control + Qt::Key::Key_End
        else if (event->matches(QKeySequence::StandardKey::MoveToEndOfDocument))
        {
            verticalScrollToEnsureVisible(self);
            const int lineIndex = self.m_docView.lastVisibleLineIndex();
            const QString& lastLine = getLine(self, lineIndex);

            if (lineIndex != self.m_lineIndex || lastLine.length() != self.m_columnIndex)
            {
                self.m_lineIndex = lineIndex;
                self.m_columnIndex = lastLine.length();
                horizontalScrollToEnsureVisible(self, line, UpdateWidths::Yes);
                scrollToMax(self.m_textView.verticalScrollBar());
                updateAfterMove(self);
            }
        }
        // Qt::Key::Key_Up
        else if (event->matches(QKeySequence::StandardKey::MoveToPreviousLine))
        {
            const int lineIndex = self.m_docView.nextVisibleLineIndex(self.m_lineIndex, -1);
            if (lineIndex == self.m_lineIndex) {
                verticalScrollToEnsureVisible(self);
            }
            else {
                self.m_lineIndex = lineIndex;
                verticalScrollToEnsureVisible(self);
                setColumnIndexAfterVerticalMove(self);
                updateAfterMove(self);
            }
        }
        // Qt::Key::Key_Down
        else if (event->matches(QKeySequence::StandardKey::MoveToNextLine))
        {
            const int lineIndex = self.m_docView.nextVisibleLineIndex(self.m_lineIndex, +1);
            if (lineIndex == self.m_lineIndex) {
                verticalScrollToEnsureVisible(self);
            }
            else {
                self.m_lineIndex = lineIndex;
                verticalScrollToEnsureVisible(self);
                setColumnIndexAfterVerticalMove(self);
                updateAfterMove(self);
            }
        }
        // Qt::Key::Key_PageUp
        else if (event->matches(QKeySequence::StandardKey::MoveToPreviousPage))
        {
            const int fullyVisibleLineCount = self.m_textView.verticalScrollBar()->pageStep();
            const int lineIndex =
                self.m_docView.nextVisibleLineIndex(self.m_lineIndex, -1 * fullyVisibleLineCount);

            verticalScrollToEnsureVisible(self);

            if (lineIndex != self.m_lineIndex)
            {
                scrollPage(self.m_textView.verticalScrollBar(), ScrollDirection::Up);
                self.m_lineIndex = lineIndex;
                setColumnIndexAfterVerticalMove(self);
                updateAfterMove(self);
            }
        }
        // Qt::Key::Key_PageDown
        else if (event->matches(QKeySequence::StandardKey::MoveToNextPage))
        {
            const int fullyVisibleLineCount = self.m_textView.verticalScrollBar()->pageStep();
            const int lineIndex = self.m_docView.nextVisibleLineIndex(self.m_lineIndex, fullyVisibleLineCount);
            verticalScrollToEnsureVisible(self);

            if (lineIndex != self.m_lineIndex)
            {
                scrollPage(self.m_textView.verticalScrollBar(), ScrollDirection::Down);
                self.m_lineIndex = lineIndex;
                setColumnIndexAfterVerticalMove(self);
                updateAfterMove(self);
            }
        }
        else {
            // See: bool QKeyEvent::matches(QKeySequence::StandardKey matchKey) const
            // "The keypad and group switch modifier should not make a difference"
            const uint searchkey = (event->modifiers() | event->key()) & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
            if (searchkey == (Qt::Modifier::ALT | Qt::Key::Key_Home))
            {
                verticalScrollToEnsureVisible(self);
                const int lineIndex = self.m_docView.firstVisibleLineIndex();

                if (lineIndex != self.m_lineIndex)
                {
                    self.m_lineIndex = lineIndex;
                    scrollToMin(self.m_textView.verticalScrollBar());
                    updateAfterMove(self);
                }
            }
            else if (searchkey == (Qt::Modifier::ALT | Qt::Key::Key_End))
            {
                verticalScrollToEnsureVisible(self);
                const int lineIndex = self.m_docView.lastVisibleLineIndex();

                if (lineIndex != self.m_lineIndex)
                {
                    self.m_lineIndex = lineIndex;
                    scrollToMax(self.m_textView.verticalScrollBar());
                    updateAfterMove(self);
                }
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
//            }
            else {
                stopEventPropagation = StopEventPropagation::No;
            }
        }
        return stopEventPropagation;
    }

    static void
    verticalScrollToEnsureVisible(TextViewTextCursor& self)
    {
        if (self.m_lineIndex < self.m_textView.firstVisibleLineIndex())
        {
            scrollToLineIndex(self, self.m_lineIndex);
        }
        else if (self.m_lineIndex > self.m_textView.lastFullyVisibleLineIndex())
        {
            QScrollBar* const vbar = self.m_textView.verticalScrollBar();
            const int vbarValue = std::max(0, self.m_lineIndex - (vbar->pageStep() - 1));
            scrollToLineIndex(self, vbarValue);
        }
    }

    static void
    setColumnIndexAfterVerticalMove(TextViewTextCursor& self)
    {
        if (0 == self.m_columnIndex) {
            scrollToMin(self.m_textView.horizontalScrollBar());
            return;
        }
        const std::vector<QString>& textLineVec = self.m_docView.doc().lineVec();
        const QString& line = textLineVec[self.m_lineIndex];
        if (line.isEmpty())
        {
            self.m_columnIndex = 0;
            horizontalScrollToEnsureVisible(self, line, UpdateWidths::No);
        }
        else {
            const QFontMetricsF fmf{self.m_textView.font()};
            int low = 0;
            int high = line.length();
            const qreal prevWidth = self.m_textBeforeWidth + 0.5 * self.m_chWidth;
            qreal width = 0;
            while (low < high) {
                const int len = midPointRoundUp(low, high);
                const QString& s = line.mid(low, len);
                const qreal w = fmf.horizontalAdvance(s);
                if (width + w < prevWidth) {
                    width += w;
                    low += len;
                }
                // Remember: When comparing qreals, operator== is unsafe.
                else {  // if (w + width >= prevWidth)
                    high = low + (len - 1);
                }
            }
            self.m_columnIndex = low;
            horizontalScrollToEnsureVisible(self, line, UpdateWidths::No);
        }
    }

    /**
     * Ex: low=0, high=4, mid=2
     * Ex: low=0, high=5, mid=3
     * Ex: low=3, high=4, mid=1
     * Ex: low=4, high=5, mid=1
     */
    inline static int
    midPointRoundUp(const int low, const int high)
    {
        // Integer math always truncates -- round down.
        const int midPointRoundDown = (high - low) / 2;
        // If width is odd, round up.
        const int isWidthOdd = (high - low) % 2;
        const int x = midPointRoundDown + isWidthOdd;
        return x;
    }

    enum class UpdateWidths { Yes, No };

    static void
    horizontalScrollToEnsureVisible(TextViewTextCursor& self, const QString& line, UpdateWidths updateWidths)
    {
        const QFontMetricsF fmf{self.m_textView.font()};
        const qreal x = fmf.horizontalAdvance(line, self.m_columnIndex);
        const QChar ch = (self.m_columnIndex < line.length()) ? line[self.m_columnIndex] : QLatin1Char{' '};
        const qreal chWidth = fmf.horizontalAdvance(ch);
        QScrollBar* const hbar = self.m_textView.horizontalScrollBar();
        if (x < hbar->value()) {
            // scroll left
            scrollToValue(hbar, x);
        }
        else if (x + chWidth > hbar->value() + self.m_textView.viewport()->width()) {
            // scroll right
            const int hbarValue = x + chWidth - self.m_textView.viewport()->width();
            scrollToValue(hbar, hbarValue);
        }
        if (UpdateWidths::Yes == updateWidths) {
            self.m_textBeforeWidth = x;
            self.m_chWidth = chWidth;
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
        const int normalisedLineIndex = self.m_docView.findNormalisedLineIndex(lineIndex);
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
};

// public
TextViewTextCursor::
TextViewTextCursor(TextView& textView, const TextViewDocumentView& docView)
    : Base{&textView},
      m_textView{textView},
      m_docView{docView},
      // Intentional: Below, immediately call slotSetBlinking(true)
      m_isBlinking{false},
      m_blinkMillis{qApp->cursorFlashTime()},
      m_isVisible{true},
      m_lineIndex{0},
      m_columnIndex{0},
      m_textBeforeWidth{0},
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
QChar
TextViewTextCursor::
chr()
const
{
    // TODO: Cache into new data member?
    const std::vector<QString>& textLineVec = m_docView.doc().lineVec();
    if (textLineVec.empty()) {
        return QLatin1Char{' '};
    }
    else {
        const QString& line = textLineVec[m_lineIndex];
        if (m_columnIndex >= line.size()) {
            return QLatin1Char{' '};
        }
        else {
            const QChar x = line[m_columnIndex];
            return x;
        }
    }
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
    }
    // Intentional: Never block processing by intended target.
    return false;
}

// public
void
TextViewTextCursor::
afterPaintEvent()
{
    m_isUpdate = false;
    m_hasMoved = false;
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
