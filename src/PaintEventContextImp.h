//
// Created by kca on 25/6/2020.
//

#ifndef SDV_PAINTEVENTCONTEXTIMP_H
#define SDV_PAINTEVENTCONTEXTIMP_H

#include "PaintEventContext.h"
#include <QFont>

namespace SDV {

class PaintEventContextImp : public PaintEventContext
{
public:
    ~PaintEventContextImp() override = default;
    void update(const QWidget& widget) override;
    const QFont& boldFont() const { return m_boldFont; }

private:
    QFont m_boldFont;
};

}  // namespace SDV

#endif //SDV_PAINTEVENTCONTEXTIMP_H
