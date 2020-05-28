//
// Created by kca on 11/4/2020.
//

#include "FindWidget.h"
#include <QLabel>
#include <QToolBar>
#include <QToolButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QDebug>
#include <QEvent>
#include "BalloonTip.h"

namespace SDV {

struct FindWidget::Private
{
    enum class EmitSignals { Yes, No };

    static void
    textChanged(FindWidget& self, EmitSignals emitSignals)
    {
        if (self.m_regexCheckBox->isChecked()) {
            const bool prevIsValid = self.m_regex.isValid();
            self.m_regex.setPattern(self.m_findLineEdit->text());
            const bool isValid = self.m_regex.isValid();
            if (isValid) {
                if ( ! prevIsValid) {
                    QPalette palette = self.m_findLineEdit->palette();
                    palette.setColor(QPalette::ColorRole::Base, QColor{Qt::GlobalColor::white});
                    self.m_findLineEdit->setPalette(palette);
                    BalloonTip::hideBalloon();
                }
            }
            else {
                if (prevIsValid) {
                    QPalette palette = self.m_findLineEdit->palette();
                    const QColor& colorRose = QColor{255, 228, 225};
                    palette.setColor(QPalette::ColorRole::Base, colorRose);
                    self.m_findLineEdit->setPalette(palette);
                }
                const QString& text = QString("Offset %1: %2").arg(self.m_regex.patternErrorOffset()).arg(self.m_regex.errorString());
                QPoint point = self.m_findLineEdit->pos();
                point.ry() += self.m_findLineEdit->height();
                const int timeoutMillis = std::numeric_limits<int>::max();
                BalloonTip::showBalloon(
                    QIcon{}, "Regex Error", text, self.m_findLineEdit->mapToGlobal(point),
                    timeoutMillis, true);
            }
        }
        if (EmitSignals::Yes == emitSignals) {
            afterUpdate(self);
        }
    }

    static void
    afterUpdate(FindWidget& self)
    {
        emit self.signalQueryChanged(
            self.m_findLineEdit->text(), self.m_matchCaseCheckBox->isChecked(), self.m_regexCheckBox->isChecked());
    }

    static void
    slotSpecialKeyPressed(FindWidget& self, const FindLineEdit::KeySequence keySequence)
    {
        if (FindLineEdit::KeySequence::Escape == keySequence) {
            self.animatedHide();
        }
        else if (FindLineEdit::KeySequence::Enter == keySequence) {
            emit self.signalNextMatch();
        }
        else if (FindLineEdit::KeySequence::ShiftEnter == keySequence) {
            emit self.signalPrevMatch();
        }
        else {
            emit self.signalSpecialKeyPressed(keySequence);
        }
    }

    static void
    slotTextChanged(FindWidget& self)
    {
        Private::textChanged(self, Private::EmitSignals::Yes);
    }

    static void
    slotMatchCaseCheckBoxStateChanged(FindWidget& self, int /*state*/)
    {
        Private::afterUpdate(self);
    }

    static void
    slotRegexCheckBoxStateChanged(FindWidget& self, int /*state*/)
    {
        slotTextChanged(self);
    }

    static void
    slotFineLineEditFocusIn(FindWidget& self)
    {
        Private::textChanged(self, Private::EmitSignals::No);
    }

    static void
    slotFineLineEditFocusOut(FindWidget& self)
    {
        BalloonTip::hideBalloon();
    }
};

// public explicit
FindWidget::
FindWidget(QWidget* parent /*= nullptr*/)
    : Base{parent}
{
    QLabel* findLabel = new QLabel{"Find:"};

    m_findLineEdit = new FindLineEdit{};
    QObject::connect(m_findLineEdit, &FindLineEdit::signalSpecialKeyPressed,
                     [this](FindLineEdit::KeySequence keySequence)
                     { Private::slotSpecialKeyPressed(*this, keySequence); });
    QObject::connect(m_findLineEdit, &FindLineEdit::textChanged,
                     [this]() { Private::slotTextChanged(*this); });
    QObject::connect(m_findLineEdit, &FindLineEdit::signalFocusIn,
                     [this]() { Private::slotFineLineEditFocusIn(*this); });
    QObject::connect(m_findLineEdit, &FindLineEdit::signalFocusOut,
                     [this]() { Private::slotFineLineEditFocusOut(*this); });

    m_matchCountLabel = new QLabel{};
    m_matchCountLabel->setToolTip("Match count");

    QToolBar* toolBar = new QToolBar{};
    toolBar->setFloatable(false);

    m_nextMatchToolButton = new QToolButton{};
    m_nextMatchToolButton->setText("ᐯ");
    m_nextMatchToolButton->setToolTip("Next Match [Enter]");
    QObject::connect(m_nextMatchToolButton, &QToolButton::clicked, this, &FindWidget::signalNextMatch);
    toolBar->addWidget(m_nextMatchToolButton);

    m_prevMatchToolButton = new QToolButton{};
    m_prevMatchToolButton->setText("ᐱ");
    m_prevMatchToolButton->setToolTip("Previous Match [Shift+Enter]");
    QObject::connect(m_prevMatchToolButton, &QToolButton::clicked, this, &FindWidget::signalPrevMatch);
    toolBar->addWidget(m_prevMatchToolButton);

    m_matchCaseCheckBox = new QCheckBox{"Match &Case"};
    // Intentional: Do not accept focus by clicking -- only tab.
    m_matchCaseCheckBox->setFocusPolicy(Qt::FocusPolicy::TabFocus);
    QObject::connect(m_matchCaseCheckBox, &QCheckBox::stateChanged,
                     [this](int state) { Private::slotMatchCaseCheckBoxStateChanged(*this, state); });

    m_regexCheckBox = new QCheckBox{"Rege&x"};
    // Intentional: Do not accept focus by clicking -- only tab.
    m_regexCheckBox->setFocusPolicy(Qt::FocusPolicy::TabFocus);
    QObject::connect(m_regexCheckBox, &QCheckBox::stateChanged,
                     [this](int state) { Private::slotRegexCheckBoxStateChanged(*this, state); });

    QHBoxLayout* layout = new QHBoxLayout{};
    layout->setContentsMargins(5, 0, 5, 0);
    layout->addWidget(findLabel);
    layout->addWidget(m_findLineEdit);
    layout->addWidget(m_matchCountLabel);
    layout->addWidget(toolBar);
    layout->addWidget(m_matchCaseCheckBox);
    layout->addWidget(m_regexCheckBox);
    setLayout(layout);
}

// protected
void
FindWidget::
showEvent(QShowEvent* event)  // override
{
    m_findLineEdit->setFocus();
    Private::textChanged(*this, Private::EmitSignals::No);
    QWidget::showEvent(event);
}

// protected
void
FindWidget::
hideEvent(QHideEvent* event)  // override
{
    // Escape to hide this widget will *not* trigger a focus-out event.  Bizarre!
    BalloonTip::hideBalloon();
    QWidget::hideEvent(event);
}

// protected
bool
FindWidget::
event(QEvent* event)  // override
{
    if (QEvent::Type::WindowDeactivate == event->type()) {
        BalloonTip::hideBalloon();
    }
    else if (QEvent::Type::WindowActivate == event->type()) {
        if (m_findLineEdit->hasFocus()) {
            Private::textChanged(*this, Private::EmitSignals::No);
        }
    }
    return QWidget::event(event);
}

}  // namespace SDV
