//
// Created by kca on 28/7/2020.
//

#include "BaseStatusBar.h"
#include <QLayout>
#include <QObjectCleanupHandler>
#include <QSizeGrip>
#include <QResizeEvent>
#include "Constants.h"
#include "QSizeGrips.h"

namespace SDV {

// public explicit
BaseStatusBar::
BaseStatusBar(QWidget* parent /*= nullptr*/)
    : Base{parent}
{
    setSizeGripEnabled(false);
    // Ref: https://stackoverflow.com/a/10439207/257299
    QObjectCleanupHandler().add(layout());

    m_sizeGrip = QSizeGrips::createWithStyle(this, Constants::Style::kFusion);
}

// public
void
BaseStatusBar::
resizeEvent(QResizeEvent* event)  // override
{
    Base::resizeEvent(event);

    // Absolutely required?  Unsure.
    m_sizeGrip->raise();

    // Based upon new size, move QSizeGrip to bottom left.
    // TODO: Support right-to-left layouts (Arabic, Hebrew, etc.)
    const QSize& newSize = event->size();
    m_sizeGrip->move(
        QPoint{
            newSize.width() - m_sizeGrip->width(),
            newSize.height() - m_sizeGrip->height()
        });
}

}  // namespace SDV
