//
// Created by kca on 25/6/2020.
//

#include "LineSegment.h"
#include <algorithm>

namespace SDV {

bool
LineSegment::
isOverlapping(const LineSegment& rhs)
const
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

}  // namespace SDV
