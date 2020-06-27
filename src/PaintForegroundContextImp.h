//
// Created by kca on 26/6/2020.
//

#ifndef SDV_PAINTFOREGROUNDCONTEXTIMP_H
#define SDV_PAINTFOREGROUNDCONTEXTIMP_H

#include "PaintContext.h"
#include <QFont>

namespace SDV {

class PaintForegroundContextImp : public PaintContext
{
public:
    ~PaintForegroundContextImp() override = default;
    void update(const QWidget& widget) override;
    const QFont& boldFont() const { return m_boldFont; }

private:
    QFont m_boldFont;
};

}  // namespace SDV

#endif //SDV_PAINTFOREGROUNDCONTEXTIMP_H
