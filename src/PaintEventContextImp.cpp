//
// Created by kca on 25/6/2020.
//

#include "PaintEventContextImp.h"
#include <QWidget>

namespace SDV {

// public
void
PaintEventContextImp::
update(const QWidget& widget)  // override
{
    m_boldFont = widget.font();
    m_boldFont.setBold(true);
}

}  // namespace SDV
