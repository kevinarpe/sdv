//
// Created by kca on 28/7/2020.
//

#include "QLabels.h"
#include <QLabel>

namespace SDV {

// public static
void
QLabels::
setSelectable(QLabel* const label)
{
    // Ref: https://stackoverflow.com/a/17957750/257299
    label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
    label->setCursor(QCursor{Qt::CursorShape::IBeamCursor});
}

}  // namespace SDV
