//
// Created by kca on 19/4/2020.
//

#include "GoToWidget.h"
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QEvent>
#include "BalloonTip.h"

static const int ITEM_INDEX_LINE_COLUMN = 0;
static const QString ITEM_TEXT_LINE_COLUMN = "Line[:Column]";
static const int ITEM_INDEX_OFFSET = 1;
static const QString ITEM_TEXT_OFFSET = "Offset";

namespace SDV {

struct GoToWidget::Private
{
    static void
    slotSpecialKeyPressed(GoToWidget& self, const FindLineEdit::KeySequence keySequence)
    {
        if (FindLineEdit::KeySequence::Escape == keySequence) {
            self.animatedHide();
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

    static QRegExp LINE_COLUMN_REGEX;  // {"^[1-9]\\d*(?::([1-9]\\d*))?$"}
    static QRegExp OFFSET_REGEX;  // {"^(?:0|[1-9]\\d*)$"}

    static void
    slotTextChanged(GoToWidget& self)
    {
        // TODO: Update m_rangeLabel
        // TODO: Handle emit -1 gracefully.
//        Private::textChanged(self, Private::EmitSignals::Yes);
        const QString& findText = self.m_findLineEdit->text().trimmed();
        if (ITEM_INDEX_LINE_COLUMN == self.m_comboBox->currentIndex()) {
            if (findText.isEmpty()) {
                // Intentional: We need to clear visual indicators if goto find text is empty.
                emit self.signalGoToLineColumn(-1, -1);
                return;
            }
            if (false == LINE_COLUMN_REGEX.exactMatch(findText)) {
                int dummy = 1;
                return;
            }
            const QString& lineNumStr = LINE_COLUMN_REGEX.cap(1);
            bool ok = false;
            // Ex: 1, 2, 3, ...
            const int lineNum = lineNumStr.toInt(&ok);
            if (!ok) {
                int dummy = 1;
                return;
            }
            int columnNum = -1;
            // Always true!
            // if (2 == LINE_COLUMN_REGEX.captureCount()) {
            // @EmptyStringAllowed
            const QString& columnNumStr = LINE_COLUMN_REGEX.cap(2);
            if (false == columnNumStr.isEmpty()) {
                columnNum = columnNumStr.toInt(&ok);
                if (!ok) {
                    int dummy = 1;
                    return;
                }
            }
            emit self.signalGoToLineColumn(lineNum, columnNum);
            return;
        }
        else if (ITEM_INDEX_OFFSET == self.m_comboBox->currentIndex()) {
            if (findText.isEmpty()) {
                // Intentional: We need to clear visual indicators if goto find text is empty.
                emit self.signalGoToOffset(-1);
                return;
            }
            if (false == OFFSET_REGEX.exactMatch(findText)) {
                int dummy = 1;
                return;
            }
            bool ok = false;
            // Ex: 0, 1, 2, ...
            const int offset = findText.toInt(&ok);
            if (!ok) {
                int dummy = 1;
                return;
            }
            emit self.signalGoToOffset(offset);
            return;
        }
        else {
            assert(false && self.m_comboBox->currentIndex());
        }
    }
};

// public static
QRegExp GoToWidget::Private::LINE_COLUMN_REGEX{"^([1-9]\\d*)(?::([1-9]\\d*))?$"};

// public static
QRegExp GoToWidget::Private::OFFSET_REGEX{"^(?:0|[1-9]\\d*)$"};

// public explicit
GoToWidget::
GoToWidget(QWidget* parent /*= nullptr*/)
    : Base{parent}
{
    QLabel* goToLabel = new QLabel{"Go To:"};

    m_comboBox = new QComboBox{};
    m_comboBox->addItem(ITEM_TEXT_LINE_COLUMN);
    m_comboBox->addItem(ITEM_TEXT_OFFSET);
    // TODO: Handle signal currentIndexChanged(int index) to re-emit as necessary

    m_findLineEdit = new FindLineEdit{};
//    m_findLineEdit->setValidator(new QIntValidator{1, 100, this});
//    m_findLineEdit->setMaximumWidth()
    QObject::connect(m_findLineEdit, &FindLineEdit::signalSpecialKeyPressed,
        [this](FindLineEdit::KeySequence keySequence) { Private::slotSpecialKeyPressed(*this, keySequence); });
    QObject::connect(m_findLineEdit, &FindLineEdit::textChanged,
        [this]() { Private::slotTextChanged(*this); });

    m_rangeLabel = new QLabel{};

    QHBoxLayout* layout = new QHBoxLayout{};
    layout->setContentsMargins(5, 0, 5, 0);
    layout->addWidget(goToLabel);
    layout->addWidget(m_comboBox);
    layout->addWidget(m_findLineEdit);
    layout->addWidget(m_rangeLabel);
    setLayout(layout);
}

// protected
void
GoToWidget::
showEvent(QShowEvent* event)  // override
{
    m_findLineEdit->setFocus();
//    Private::textChanged(*this, Private::EmitSignals::No);
    QWidget::showEvent(event);
}

// protected
void
GoToWidget::
hideEvent(QHideEvent* event)  // override
{
    // Escape to hide this widget will *not* trigger a focus-out event.  Bizarre!
    BalloonTip::hideBalloon();
    QWidget::hideEvent(event);
}

// protected
bool
GoToWidget::
event(QEvent* event)  // override
{
    if (QEvent::Type::WindowDeactivate == event->type()) {
        BalloonTip::hideBalloon();
    }
    else if (QEvent::Type::WindowActivate == event->type()) {
        if (m_findLineEdit->hasFocus()) {
//            Private::textChanged(*this, Private::EmitSignals::No);
        }
    }
    return QWidget::event(event);
}

}  // namespace SDV
