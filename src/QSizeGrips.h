//
// Created by kca on 28/7/2020.
//

#ifndef SDV_QSIZEGRIPS_H
#define SDV_QSIZEGRIPS_H

#include <QString>
class QSizeGrip;
class QWidget;

namespace SDV {

struct QSizeGrips
{
    /**
     * @param style
     *        see {@link Constants::Style::kFusion}
     */
    static QSizeGrip*
    createWithStyle(QWidget* parent, const QString& style);
};

}  // namespace SDV

#endif //SDV_QSIZEGRIPS_H
