//
// Created by kca on 19/4/2020.
//

#include "StatusBar.h"
#include <QLabel>
#include <QVBoxLayout>

namespace SDV {

// public explicit
StatusBar::
StatusBar(QWidget* parent /*= nullptr*/)
    : Base{parent}
{
    m_sharedLabel = new QLabel{};
    // Ref: https://stackoverflow.com/a/17957750/257299
    m_sharedLabel->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
    m_sharedLabel->setCursor(QCursor{Qt::CursorShape::IBeamCursor});

    m_textViewLabel = new QLabel{};
    m_textViewLabel->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
    m_textViewLabel->setCursor(QCursor{Qt::CursorShape::IBeamCursor});

    QVBoxLayout* vboxLayout = new QVBoxLayout{};
    vboxLayout->addWidget(m_sharedLabel);
    vboxLayout->addWidget(m_textViewLabel);

    QWidget* statusBarWidget = new QWidget{};
    statusBarWidget->setLayout(vboxLayout);
    addWidget(statusBarWidget);
}

}  // namespace SDV
