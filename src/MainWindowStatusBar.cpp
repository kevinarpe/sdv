//
// Created by kca on 19/4/2020.
//

#include "MainWindowStatusBar.h"
#include <QLabel>
#include <QSizeGrip>
#include <QStyleFactory>
#include <QBoxLayout>
#include <QResizeEvent>
#include "QLabels.h"

namespace SDV {

// public explicit
MainWindowStatusBar::
MainWindowStatusBar(QWidget* parent /*= nullptr*/)
    : Base{parent}
{
    m_nodePathLabel = new QLabel{this};
    QLabels::setSelectable(m_nodePathLabel);

    m_textCursorPositionLabel = new QLabel{this};
    QLabels::setSelectable(m_textCursorPositionLabel);

    m_textStatsLabel = new QLabel{this};
    QLabels::setSelectable(m_textStatsLabel);

    QHBoxLayout* const hbox = new QHBoxLayout{};
    hbox->addWidget(m_nodePathLabel);
    hbox->addStretch();
    hbox->addWidget(m_textCursorPositionLabel);

    QVBoxLayout* const vbox = new QVBoxLayout{};
    vbox->addLayout(hbox);
    vbox->addWidget(m_textStatsLabel);

    setLayout(vbox);
    slotSetTextCursorPosition(0, 0);
}

// public slot
void
MainWindowStatusBar::
slotSetTextCursorPosition(const int lineIndex, const int columnIndex)
{
    const QString& text = QString{QLatin1String{"Line %1, Column %2"}}.arg(1 + lineIndex).arg(1 + columnIndex);
    m_textCursorPositionLabel->setText(text);
}

}  // namespace SDV
