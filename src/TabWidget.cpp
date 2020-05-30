//
// Created by kca on 4/4/2020.
//

#include "TabWidget.h"

namespace SDV {

struct TabWidget::Private
{
    static void
    slotNextTab(TabWidget& self)
    {
        const int count = self.count();
        if (0 == count) {
            return;
        }
        const int index = self.currentIndex();
        if (index == count - 1) {
            self.setCurrentIndex(0);
        }
        else {
            self.setCurrentIndex(index + 1);
        }
    }

    static void
    slotPrevTab(TabWidget& self)
    {
        const int count = self.count();
        if (0 == count) {
            return;
        }
        const int index = self.currentIndex();
        if (index == 0) {
            self.setCurrentIndex(count - 1);
        }
        else {
            self.setCurrentIndex(index - 1);
        }
    }
};

// public
TabWidget::
TabWidget(QWidget* parent /*= nullptr*/)
    : Base{parent}
{
    // Ref: https://stackoverflow.com/a/17631983/257299
    QShortcut* nextTabShortcut = new QShortcut(QKeySequence{Qt::CTRL + Qt::Key_PageDown}, this);
    // Ref: https://stackoverflow.com/a/45832071/257299
    QObject::connect(nextTabShortcut, &QShortcut::activated, [this]() { Private::slotNextTab(*this); });

    QShortcut* prevTabShortcut = new QShortcut(QKeySequence{Qt::CTRL + Qt::Key_PageUp}, this);
    QObject::connect(prevTabShortcut, &QShortcut::activated, [this]() { Private::slotPrevTab(*this); });
}

}  // namespace SDV
