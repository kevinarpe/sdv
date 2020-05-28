//
// Created by kca on 24/5/2020.
//

#include "SmoothShowHideWidget.h"

namespace SDV {

struct SmoothShowHideWidget::Private
{
    static void
    createShowAnimation(SmoothShowHideWidget& self)
    {
        if (nullptr != self.m_nullableShowAnimation) {
            return;
        }
        // Ref: https://stackoverflow.com/a/24106492/257299
        self.m_nullableShowAnimation = std::make_unique<QPropertyAnimation>(&self, "maximumHeight", &self);
        self.m_nullableShowAnimation->setDuration(100);
        self.m_nullableShowAnimation->setStartValue(0);
    }

    static void
    createHideAnimation(SmoothShowHideWidget& self)
    {
        if (nullptr != self.m_nullableHideAnimation) {
            return;
        }
        // Ref: https://stackoverflow.com/a/24106492/257299
        self.m_nullableHideAnimation = std::make_unique<QPropertyAnimation>(&self, "maximumHeight", &self);
        self.m_nullableHideAnimation->setDuration(100);
        self.m_nullableHideAnimation->setEndValue(0);
        QObject::connect(self.m_nullableHideAnimation.get(), &QPropertyAnimation::finished, &self, &QWidget::hide);
    }
};

// public explicit
SmoothShowHideWidget::
SmoothShowHideWidget(QWidget* parent /*= nullptr*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/)
    : Base{parent, f}, m_height{-1}
{}

// public slot
void
SmoothShowHideWidget::
setVisible(const bool visible)  // override
{
    const bool prevVisible = isVisible();
    QWidget::setVisible(visible);
    if (visible != prevVisible) {
        if (nullptr != m_nullableShowAnimation) {
            m_nullableShowAnimation->stop();
        }
        if (nullptr != m_nullableHideAnimation) {
            m_nullableHideAnimation->stop();
        }
    }
    // Ref: https://www.qtcentre.org/threads/60494-Animate-an-hidden-widget
    if (visible) {
        if (-1 == m_height) {
            m_height = height();
        }
        Private::createShowAnimation(*this);
        m_nullableShowAnimation->setEndValue(m_height);
        m_nullableShowAnimation->start(QAbstractAnimation::DeletionPolicy::KeepWhenStopped);
    }
    else {
        emit signalHidden();
    }
}

// public slot
void
SmoothShowHideWidget::
animatedHide()
{
    if (!isVisible()) {
        return;
    }
    if (nullptr != m_nullableShowAnimation) {
        m_nullableShowAnimation->stop();
    }
    Private::createHideAnimation(*this);
    m_nullableHideAnimation->stop();
    m_nullableHideAnimation->setStartValue(m_height);
    m_nullableHideAnimation->start(QAbstractAnimation::DeletionPolicy::KeepWhenStopped);
}

}  // namespace SDV
