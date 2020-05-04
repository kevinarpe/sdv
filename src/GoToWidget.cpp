//
// Created by kca on 19/4/2020.
//

#include "GoToWidget.h"
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>

static QString ITEM_TEXT_LINE_COLUMN = "Line[:Column]";
static QString ITEM_TEXT_OFFSET = "Offset";

namespace SDV {

struct GoToWidget::Private
{
    static void
    slotSpecialKeyPressed(GoToWidget& self, const FindLineEdit::KeySequence keySequence)
    {
        if (FindLineEdit::KeySequence::Escape == keySequence) {
            self.hide();
            emit self.signalHidden();
        }
        else if (FindLineEdit::KeySequence::Enter == keySequence) {
            // Intentional: Do nothing -- do not emit signal below.
            int dummy = 1;
        }
        else if (FindLineEdit::KeySequence::ShiftEnter == keySequence) {
            // Intentional: Do nothing -- do not emit signal below.
            int dummy = 1;
        }
        else {
            emit self.signalSpecialKeyPressed(keySequence);
        }
    }
};

// public explicit
GoToWidget::
GoToWidget(QWidget* parent /*= nullptr*/)
    : QWidget{parent}
{
    QLabel* goToLabel = new QLabel{"Go To:"};

    m_comboBox = new QComboBox{};
    m_comboBox->addItem(ITEM_TEXT_LINE_COLUMN);
    m_comboBox->addItem(ITEM_TEXT_OFFSET);

    m_findLineEdit = new FindLineEdit{};
    QObject::connect(m_findLineEdit, &FindLineEdit::signalSpecialKeyPressed,
        [this](FindLineEdit::KeySequence keySequence) { Private::slotSpecialKeyPressed(*this, keySequence); });

    m_rangeLabel = new QLabel{};

    QHBoxLayout* layout = new QHBoxLayout{};
    layout->setContentsMargins(5, 0, 5, 0);
    layout->addWidget(goToLabel);
    layout->addWidget(m_comboBox);
    layout->addWidget(m_findLineEdit);
    layout->addWidget(m_rangeLabel);
    setLayout(layout);
}

}  // namespace SDV