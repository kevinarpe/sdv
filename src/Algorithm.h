//
// Created by kca on 31/5/2020.
//

#ifndef SDV_ALGORITHM_H
#define SDV_ALGORITHM_H

#include <cstddef>
//#include <concepts>

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

    /**
     * @param __defaultValue
     *        Intentional: Value, not (const) reference!  Why?  Reference to temporary is dangerous.
     */
    template<typename _Container, typename _Value>
    static _Value
    frontOrDefault(const _Container& __c, _Value __defaultValue = _Value{})
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

    /**
     * @param __defaultValue
     *        Intentional: Value, not (const) reference!  Why?  Reference to temporary is dangerous.
     */
    template<typename _Container, typename _Value>
    static _Value
    backOrDefault(const _Container& __c, _Value __defaultValue = _Value{})
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
    // Wait for GCC 10
//
//    // Ref: https://en.cppreference.com/w/cpp/concepts/integral
//    // Ref: https://www.modernescpp.com/index.php/c-core-guidelines-rules-for-the-usage-of-concepts-2
//
//    // Concepts Style A:
////    template<typename _Integral>
////    requires std::integral<_Integral>
//
//    // Concepts Style B:
//    template<std::integral _Integral>
//    static _Integral
//    midPointRoundUp(const _Integral __low, const _Integral __high)
//    {
//        assert(__low <= __high);
//        // Integer math always truncates -- round down.
//        const _Integral midPointRoundDown = (__high - __low) / 2;
//        // If width is odd, round up.
//        const _Integral isRangeOdd = (__high - __low) % 2;
//        const _Integral x = midPointRoundDown + isRangeOdd;
//        return x;
//    }

    struct Vector
    {
        /**
         * @param __defaultValue
         *        Intentional: Value, not (const) reference!  Why?  Reference to temporary is dangerous.
         */
        template<typename _Vector, typename _Value>
        static _Value
        valueOrDefault(const _Vector& vec, typename _Vector::size_type pos, _Value __defaultValue = _Value{})
        {
            if (pos < vec.size()) {
                const _Value& x = vec[pos];
                return x;
            }
            else {
                return __defaultValue;
            }
        }
    };

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
