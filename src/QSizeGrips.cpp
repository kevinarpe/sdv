//
// Created by kca on 28/7/2020.
//

#include "QSizeGrips.h"
#include <QSizeGrip>
#include <QStyleFactory>

namespace SDV {

// public static
QSizeGrip*
QSizeGrips::
createWithStyle(QWidget* parent, const QString& style)
{
    QSizeGrip* sizeGrip = new QSizeGrip{parent};
    QStyle* const nullableStyle = QStyleFactory::create(style);
    if (nullptr != nullableStyle) {
        sizeGrip->setStyle(nullableStyle);
    }
    const QSize& size = sizeGrip->size();
    const int edge = qMin(size.width(), size.height());
    sizeGrip->setMinimumSize(QSize{edge, edge});
    sizeGrip->setMaximumSize(QSize{edge, edge});
    return sizeGrip;
}

}  // namespace SDV
