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
        int __index = 0;
        for (typename _Container::const_iterator __iter = __c.cbegin()
            ; __c.cend() != __iter
            ; ++__iter, ++__index)
        {
            if (__p(*__iter)) {
                return __index;
            }
        }
        return -1;
    }

    template<typename _Container, typename _Value>
    static _Value
    frontOrDefault(const _Container& __c, _Value __defaultValue)
    {
        typename _Container::const_iterator __iter = __c.cbegin();
        if (__c.cend() == __iter) {
            return __defaultValue;
        }
        else {
            const _Value& __x = *__iter;
            return __x;
        }
    }

    template<typename _Container, typename _Value>
    static _Value
    backOrDefault(const _Container& __c, _Value __defaultValue)
    {
        typename _Container::const_reverse_iterator __iter = __c.crbegin();
        if (__c.crend() == __iter) {
            return __defaultValue;
        }
        else {
            const _Value& __x = *__iter;
            return __x;
        }
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
        getOrDefault(const _Map& __map, _Key __key, _Value __defaultValue)
        {
            typename _Map::const_iterator __iter = __map.find(__key);
            if (__map.cend() == __iter) {
                return __defaultValue;
            }
            else {
                const _Value& __x = __iter->second;
                return __x;
            }
        }
    };
};

}  // namespace SDV

#endif //SDV_ALGORITHM_H
