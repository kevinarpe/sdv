//
// Created by kca on 31/5/2020.
//

#ifndef SDV_ALGORITHM_H
#define SDV_ALGORITHM_H

#include <cstddef>

namespace SDV {

struct Algorithm
{
    // Why is STL so 'iterator' focused, when sometimes indices are easier to read?  :)
    template<typename _Container, typename _Predicate>
    static std::size_t
    findFirstIndexIf(const _Container& __c, _Predicate __p)
    {
        int i = 0;
        for (typename _Container::const_iterator iter = __c.cbegin(); __c.cend() != iter; ++iter, ++i) {
            if (__p(*iter)) {
                return i;
            }
        }
        return -1;
    }
};

}  // namespace SDV

#endif //SDV_ALGORITHM_H
