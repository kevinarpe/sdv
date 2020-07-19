//
// Created by kca on 23/5/2020.
//

#ifndef SDV_GOTOLINEEDIT_H
#define SDV_GOTOLINEEDIT_H

#include <QLineEdit>

namespace SDV {

class GoToLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    using Base = QLineEdit;

    explicit GoToLineEdit(QWidget* parent = nullptr);
    ~GoToLineEdit() override = default;
};

}  // namespace SDV

#endif //SDV_GOTOLINEEDIT_H
