//
// Created by kca on 11/4/2020.
//

#include "FindLineEdit.h"
#include <QKeyEvent>

namespace SDV {

// protected
void
FindLineEdit::
keyPressEvent(QKeyEvent* event) // override
{
    QLineEdit::keyPressEvent(event);

    const Qt::KeyboardModifiers& m = event->modifiers();
    const int k = event->key();
    if (0 == m && Qt::Key::Key_Escape == k) {
        emit signalSpecialKeyPressed(KeySequence::Escape);
    }
    else if (0 == m && (Qt::Key::Key_Return == k || Qt::Key::Key_Enter == k)) {
        emit signalSpecialKeyPressed(KeySequence::Enter);
    }
    else if (Qt::KeyboardModifier::ShiftModifier == m && (Qt::Key::Key_Return == k || Qt::Key::Key_Enter == k)) {
        emit signalSpecialKeyPressed(KeySequence::ShiftEnter);
    }
    else if (0 == m && Qt::Key::Key_PageUp == k) {
        emit signalSpecialKeyPressed(KeySequence::PageUp);
    }
    else if (0 == m && Qt::Key::Key_PageDown == k) {
        emit signalSpecialKeyPressed(KeySequence::PageDown);
    }
    else if (Qt::KeyboardModifier::ControlModifier == m && Qt::Key::Key_PageUp == k) {
        emit signalSpecialKeyPressed(KeySequence::CtrlHome);
    }
    else if (Qt::KeyboardModifier::ControlModifier == m && Qt::Key::Key_PageDown == k) {
        emit signalSpecialKeyPressed(KeySequence::CtrlEnd);
    }
    else if (Qt::KeyboardModifier::AltModifier == m && Qt::Key::Key_PageUp == k) {
        emit signalSpecialKeyPressed(KeySequence::AltPageUp);
    }
    else if (Qt::KeyboardModifier::AltModifier == m && Qt::Key::Key_PageDown == k) {
        emit signalSpecialKeyPressed(KeySequence::AltPageDown);
    }
}

// protected
void
FindLineEdit::
focusInEvent(QFocusEvent* event) // override
{
    QLineEdit::focusInEvent(event);
    emit signalFocusIn();
}

// protected
void
FindLineEdit::
focusOutEvent(QFocusEvent* event) // override
{
    QLineEdit::focusOutEvent(event);
    emit signalFocusOut();
}

}  // namespace SDV
