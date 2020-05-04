//
// Created by kca on 3/5/2020.
//

#ifndef SDV_BIT_H
#define SDV_BIT_H

#include <bitset>
#include <limits>
#include <type_traits>

namespace SDV {

// Ref: https://stackoverflow.com/a/109069/257299
template<typename T>
//static inline  // static if you want to compile with -mpopcnt in one compilation unit but not others
typename std::enable_if<std::is_integral<T>::value, unsigned>::type
popcount(T value)
{
    static_assert(std::numeric_limits<T>::radix == 2, "non-binary type");

    // sizeof(value)*CHAR_BIT
    constexpr int bitwidth = std::numeric_limits<T>::digits + std::numeric_limits<T>::is_signed;
    // std::bitset constructor was only unsigned long before C++11.  Beware if porting to C++03
    static_assert(bitwidth <= std::numeric_limits<unsigned long long>::digits, "arg too wide for std::bitset() constructor");

    typedef typename std::make_unsigned<T>::type UT;        // probably not needed, bitset width chops after sign-extension

    std::bitset<bitwidth> bs( static_cast<UT>(value) );
    const size_t x = bs.count();
    return x;
}

}  // namespace SDV

#endif //SDV_BIT_H
