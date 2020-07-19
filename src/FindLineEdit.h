//
// Created by kca on 11/4/2020.
//

#ifndef SDV_FINDLINEEDIT_H
#define SDV_FINDLINEEDIT_H

#include <QLineEdit>

namespace SDV {

class FindLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    enum class KeySequence { Escape, Enter, ShiftEnter, PageUp, PageDown, CtrlHome, CtrlEnd, AltPageUp, AltPageDown };
    using Base = QLineEdit;

    explicit FindLineEdit(QWidget* parent = nullptr) : Base(parent) {}
    ~FindLineEdit() override = default;

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

signals:
    void signalSpecialKeyPressed(KeySequence keySequence);
    void signalFocusIn();
    void signalFocusOut();
};

}  // namespace SDV

#endif //SDV_FINDLINEEDIT_H
