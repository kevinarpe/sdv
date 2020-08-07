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
#include "MainWindowStatusBarNodePathLabel.h"

namespace SDV {

static void
setSunkenPanel(QLabel* const label)
{
    label->setFrameShape(QFrame::Shape::Panel);
    label->setFrameShadow(QFrame::Shadow::Sunken);
    const int lineWidth = label->lineWidth();
    const int newLineWidth = std::max(lineWidth, 2);
    label->setLineWidth(newLineWidth);
}

static void
setSelectable(QLabel* const label)
{
    QLabels::setSelectable(label);
    setSunkenPanel(label);
}

// public explicit
MainWindowStatusBar::
MainWindowStatusBar(TextView* const textView, QWidget* parent /*= nullptr*/)
    : Base{parent}
{
    // TODO: Add context menu with single item: Copy
    m_nodePathLabel = new MainWindowStatusBarNodePathLabel{textView, this};
    m_nodePathLabel->setTextFormat(Qt::TextFormat::RichText);
    setSunkenPanel(m_nodePathLabel);

    m_textCursorPositionLabel = new QLabel{};
    m_textCursorPositionLabel->setTextFormat(Qt::TextFormat::PlainText);
    setSelectable(m_textCursorPositionLabel);

    m_textStatsLabel = new QLabel{};
    m_textStatsLabel->setTextFormat(Qt::TextFormat::RichText);
    setSelectable(m_textStatsLabel);

    QHBoxLayout* const hbox = new QHBoxLayout{};
    hbox->addWidget(new QLabel{"Node Path:"});
    hbox->addWidget(m_nodePathLabel, 1);
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
