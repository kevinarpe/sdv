//
// Created by kca on 26/6/2020.
//

#include "PaintForegroundContextImp.h"
#include <QWidget>

namespace SDV {

// public
void
PaintForegroundContextImp::
update(const QWidget& widget)  // override
{
    m_boldFont = widget.font();
    m_boldFont.setBold(true);
}

}  // namespace SDV
