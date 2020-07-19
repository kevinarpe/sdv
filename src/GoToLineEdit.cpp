//
// Created by kca on 23/5/2020.
//

#include "GoToLineEdit.h"
#include <QIntValidator>

namespace SDV {

// public explicit
GoToLineEdit::
GoToLineEdit(QWidget* parent /*= nullptr*/)
    : Base(parent)
{
    setValidator(new QIntValidator{1, 100, this});
}

}  // namespace SDV
