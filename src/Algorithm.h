//
// Created by kca on 31/5/2020.
//

#ifndef SDV_ALGORITHM_H
#define SDV_ALGORITHM_H

#include <cstddef>
#include <utility>
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
        valueOrDefault(const _Vector& __vec, typename _Vector::size_type __pos, _Value __defaultValue = _Value{})
        {
            if (__pos < __vec.size()) {
                const _Value& x = __vec[__pos];
                return x;
            }
            else {
                return __defaultValue;
            }
        }

        template<typename _Vector, typename _Value>
        static typename _Vector::iterator
        findOrAssert(_Vector& __vec, const _Value& __value)
        {
            typename _Vector::iterator it = std::find(__vec.begin(), __vec.end(), __value);
            assert(__vec.end() != it);
            return it;
        }

        template<typename _Vector, typename _Value>
        static typename _Vector::const_iterator
        findOrAssert(const _Vector& __vec, const _Value& __value)
        {
            typename _Vector::const_iterator it = std::find(__vec.begin(), __vec.end(), __value);
            assert(__vec.end() != it);
            return it;
        }

        template<typename _Vector, typename _Value>
        static bool
        exists(const _Vector& __vec, const _Value& __value)
        {
            typename _Vector::const_iterator it = std::find(__vec.begin(), __vec.end(), __value);
            const bool x = (__vec.end() != it);
            return x;
        }

        template<typename _Vector, typename _Value>
        static bool
        tryPushBackUnique(_Vector& __vec, const _Value& __value)
        {
            if (exists(__vec, __value))
            {
                return false;
            }
            __vec.push_back(__value);
            return true;
        }

        template<typename _Vector, typename _Value>
        static void
        pushBackUniqueOrAssert(_Vector& __vec, const _Value& __value)
        {
            assert(tryPushBackUnique(__vec, __value));
        }

        template<typename _Vector, typename _Value>
        static void
        eraseFirstOrAssert(_Vector& __vec, const _Value& __value)
        {
            typename _Vector::iterator it = std::find(__vec.begin(), __vec.end(), __value);
            assert(__vec.end() != it);
            __vec.erase(it);
        }

        template<typename _Vector, typename _Value>
        static bool
        tryErase(_Vector& __vec, const _Value& __value)
        {
            typename _Vector::iterator it = std::find(__vec.begin(), __vec.end(), __value);
            if (__vec.end() == it) {
                return false;
            }
            else {
                __vec.erase(it);
                return true;
            }
        }
    };

    struct Set
    {
        template<typename _Set, typename _Value>
        static std::pair<typename _Set::iterator, bool>
        insertNewOrAssert(_Set& __set, const _Value& __constRefValue)
        {
            const std::pair<typename _Set::iterator, bool> r = __set.insert(__constRefValue);
            assert(r.second);
            return r;
        }

        template<typename _Set, typename _Value>
        static std::pair<typename _Set::iterator, bool>
        insertNewOrAssert(_Set& __set, _Value&& __rvalueRefValue)
        {
            const std::pair<typename _Set::iterator, bool> r = __set.insert(__rvalueRefValue);
            assert(r.second);
            return r;
        }

        template<typename _Set, typename _Value>
        static bool
        tryErase(_Set& __set, const _Value& __value)
        {
            const std::size_t count = __set.erase(__value);
            const bool x = (count > 1);
            return x;
        }

        template<typename _Set, typename _Value>
        static void
        eraseOrAsset(_Set& __set, const _Value& __value)
        {
            assert(tryErase(__set, __value));
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
        getOrDefault(const _Map& __map, const _Key& __key,
                     // Intentional: This *must* be passed by value.  Pass by reference is dangerous if caller used a temporary value.
                     // I watched a Facebook talk about C++ on YouTube a long time ago...
                     _Value __defaultValue)
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

        template<typename _Map, typename _Key>
        static const typename _Map::mapped_type&
        getOrAssert(const _Map& __constRefMap, const _Key& __key)
        {
            const typename _Map::const_iterator __iter = __constRefMap.find(__key);
            assert(__constRefMap.end() != __iter);
            const typename _Map::mapped_type& __x = __iter->second;
            return __x;
        }

        template<typename _Map, typename _Key>
        static typename _Map::mapped_type&
        getOrAssert(_Map& __refMap, const _Key& __key)
        {
            const typename _Map::iterator __iter = __refMap.find(__key);
            assert(__refMap.end() != __iter);
            typename _Map::mapped_type& __x = __iter->second;
            return __x;
        }

        template<typename _Map, typename _Value>
        static typename _Map::const_iterator
        findByValueOrAssert(const _Map& __constRefMap, const _Value& __value)
        {
            typename _Map::const_iterator __iter =
                std::find_if(__constRefMap.begin(), __constRefMap.end(),
                             [&__value](const auto& __pair){ return (__pair.second == __value); });

            assert(__constRefMap.end() != __iter);
            return __iter;
        }

        template<typename _Map, typename _Value>
        static typename _Map::const_iterator
        findByValueOrAssert(_Map& __refMap, const _Value& __value)
        {
            typename _Map::const_iterator __iter =
                std::find_if(__refMap.begin(), __refMap.end(),
                             [&__value](const auto& __pair){ return (__pair.second == __value); });

            assert(__refMap.end() != __iter);
            return __iter;
        }

        // Ref: https://en.cppreference.com/w/cpp/container/unordered_map/insert_or_assign
        template<typename _Map, typename _Key, typename _Value>
        static std::pair<typename _Map::iterator, bool>
        insertNewOrAssert(_Map& map, const _Key& __key, _Value&& __value)
        {
            std::pair<typename _Map::iterator, bool> r = map.insert_or_assign(__key, __value);
            assert(r.second);
            return r;
        }

        template<typename _Map, typename _Key>
        static void
        eraseByKeyOrAssert(_Map& map, const _Key& __key)
        {
            typename _Map::size_type eraseCount = map.erase(__key);
            assert(1 == eraseCount);
        }

        template<typename _Map, typename _Key>
        static bool
        tryEraseByKey(_Map& map, const _Key& __key)
        {
            typename _Map::size_type eraseCount = map.erase(__key);
            const bool x = (eraseCount > 0);
            return x;
        }
    };
};

}  // namespace SDV

#endif //SDV_ALGORITHM_H
