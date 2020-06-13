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

    struct Map
    {
        /**
         * @tparam _Map
         *         Ex: std::map<K, V> or std::unordered_map<K, V>
         *
         * @param key
         *        value to find
         * @param defaultValue
         *        if key is not found, return this value
         *
         * @return value mapped to key or defaultValue
         *
         * @see std::map::find(_Key)
         * @see std::unordered_map::find(_Key)
         */
        template<typename _Map, typename _Key, typename _Value>
        static _Value
        getOrDefault(const _Map& map, _Key key, _Value defaultValue)
        {
            typename _Map::const_iterator i = map.find(key);
            if (map.cend() == i) {
                return defaultValue;
            }
            else {
                const _Value& x = i->second;
                return x;
            }
        }
    };
};

}  // namespace SDV

#endif //SDV_ALGORITHM_H
