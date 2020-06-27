//
// Created by kca on 27/6/2020.
//

#ifndef SDV_LINEFORMAT_H
#define SDV_LINEFORMAT_H

#include <memory>
#include <cassert>
#include "LineSegment.h"

namespace SDV {

template<typename PaintFunctor>
struct LineFormat
{
    LineSegment seg;
    std::shared_ptr<PaintFunctor> f;

    LineFormat(const LineSegment& p_seg, const std::shared_ptr<PaintFunctor>& p_f)
        : seg{p_seg}, f{p_f}
    {
        assert(p_seg.isValid() && nullptr != p_f);
    }
};

}  // namespace SDV

#endif //SDV_LINEFORMAT_H
