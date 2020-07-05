//
// Created by kca on 5/7/2020.
//

#include "QKeyEvents.h"
#include <QKeyEvent>

namespace SDV {

// public static
bool
QKeyEvents::
matches(QKeyEvent* const event, const unsigned int key)
{
    // See: bool QKeyEvent::matches(QKeySequence::StandardKey matchKey) const
    // "The keypad and group switch modifier should not make a difference"
    const unsigned int searchkey =
        (event->modifiers() | event->key())
        &
        ~(Qt::KeyboardModifier::KeypadModifier | Qt::KeyboardModifier::GroupSwitchModifier);

    const bool x = (key == searchkey);
    return x;
}

}  // namespace SDV
