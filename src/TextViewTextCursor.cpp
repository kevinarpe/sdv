//
// Created by kca on 10/6/2020.
//

#include "TextViewTextCursor.h"
#include <QApplication>
#include <QKeyEvent>
#include <QDebug>
#include "TextView.h"
#include "TextViewDocument.h"
#include "TextViewDocumentView.h"

namespace SDV {

struct TextViewTextCursor::Private
{
    static void
    update(TextViewTextCursor& self)
    {
        self.m_isUpdate = true;
        self.m_textView.viewport()->update(self.m_textView.textCursorRect());
    }

    static void
    beforeMove(TextViewTextCursor& self)
    {
        if (false == self.m_hasMoved) {
            self.m_prevLineIndex = self.m_lineIndex;
            self.m_prevColumnIndex = self.m_columnIndex;
            self.m_hasMoved = true;
        }
    }

    static void
    afterMove(TextViewTextCursor& self)
    {
        self.m_isUpdate = true;
        self.m_textView.viewport()->update();
    }

    static void
    keyPressEvent(TextViewTextCursor& self, QKeyEvent* event)
    {
        if (event->matches(QKeySequence::StandardKey::MoveToPreviousChar))
        {
            if (0 == self.m_columnIndex)
            {
                const int prevLineIndex = self.m_docView.prevLineIndex(self.m_lineIndex);
                if (prevLineIndex != self.m_lineIndex)
                {
                    beforeMove(self);
                    self.m_lineIndex = prevLineIndex;
                    self.m_columnIndex = self.m_docView.doc().textLineVec()[prevLineIndex].length();
                    afterMove(self);
                }
            }
            else {
                beforeMove(self);
                --self.m_columnIndex;
                afterMove(self);
            }
        }
        else if (event->matches(QKeySequence::StandardKey::MoveToNextChar))
        {
            if (self.m_docView.doc().textLineVec()[self.m_lineIndex].length() == self.m_columnIndex)
            {
                const int nextLineIndex = self.m_docView.nextLineIndex(self.m_lineIndex);
                if (nextLineIndex != self.m_lineIndex)
                {
                    beforeMove(self);
                    self.m_lineIndex = nextLineIndex;
                    self.m_columnIndex = 0;
                    afterMove(self);
                }
            }
            else {
                beforeMove(self);
                ++self.m_columnIndex;
                afterMove(self);
            }
        }
    }

    static void
    focusInEvent(TextViewTextCursor& self, QFocusEvent* event)
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

    static QChar
    ch(const TextViewTextCursor& self, const int lineIndex, const int columnIndex)
    {
        const std::vector<QString>& textLineVec = self.m_docView.doc().textLineVec();
        if (textLineVec.empty()) {
            return QLatin1Char{' '};
        }
        else {
            const QString& line = textLineVec[lineIndex];
            if (columnIndex >= line.size()) {
                return QLatin1Char{' '};
            }
            else {
                const QChar x = line[columnIndex];
                return x;
            }
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
      m_isUpdate{false},
      m_hasMoved{false},
      m_prevLineIndex{-1},
      m_prevColumnIndex{-1}
{
    textView.installEventFilter(this);
    slotSetBlinking(true);
}

// public
QChar
TextViewTextCursor::
ch()
const
{
    // TODO: Cache into new data member?
    const QChar& x = Private::ch(*this, m_lineIndex, m_columnIndex);
    return x;
}

// public
QChar
TextViewTextCursor::
prevCh()
const
{
    const QChar& x = Private::ch(*this, m_prevLineIndex, m_prevColumnIndex);
    return x;
}

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
            Private::keyPressEvent(*this, static_cast<QKeyEvent*>(event));
            break;
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
    m_prevLineIndex = -1;
    m_prevColumnIndex = -1;
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
