//
// Created by kca on 25/6/2020.
//

#ifndef SDV_LINESEGMENT_H
#define SDV_LINESEGMENT_H

#include <algorithm>

namespace SDV {

struct LineSegment
{
    int charIndex;
    int length;

    bool isValid() const
    {
        const bool x = (charIndex >= 0 && length > 0);
        return x;
    }

    bool isOverlapping(const LineSegment& rhs) const
    {
        // Ref: https://stackoverflow.com/a/12888920/257299
        // Ref: https://stackoverflow.com/a/325964/257299
        const int maxBegin = std::max(charIndex, rhs.charIndex);
        const int minEnd = std::min(charIndex + length, rhs.charIndex + rhs.length);
        // Why operator< ?  Begin and end overlap is allowed.
        // Ex: [0, 2]->[0, 3) and [3, 5]->[3, 6)
        // The first segment has three indices: 0, 1, 2.  Inclusive begin = 0, inclusive end = 2, exclusive end = 3
        // The second segment has three indices: 3, 4, 5.  Inclusive begin = 3, inclusive end = 5, exclusive end = 6
        const bool x = (maxBegin < minEnd);
        return x;
    }

    /**
     * @tparam LineSegmentParent
     *         either {@link PainterBackgroundFunctor} or {@link PainterForegroundFunctor}
     */
    // Ref: https://stackoverflow.com/a/46128321/257299
    template <typename LineSegmentParent>
    struct NonOverlapCompare
    {
        bool operator()(const LineSegmentParent& lhs, const LineSegmentParent& rhs) const
        {
            const LineSegment& lhs2 = lhs.seg;
            const LineSegment& rhs2 = rhs.seg;
            if (lhs2.isOverlapping(rhs2)) {
                return false;
            }
            const bool x = (lhs2.charIndex < rhs2.charIndex);
            return x;
        }
    };
};

}  // namespace SDV

#endif //SDV_LINESEGMENT_H
