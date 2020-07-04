//
// Created by kca on 4/7/2020.
//

#ifndef SDV_COUNTINGITERATOR_H
#define SDV_COUNTINGITERATOR_H

#include <iterator>

namespace SDV {

template<typename TNumber>
class CountingIterator
{
public:
    // Ref: https://www.fluentcpp.com/2018/05/08/std-iterator-deprecated/
    // Ref: https://stackoverflow.com/a/38103394/257299
    // Ref: https://www.boost.org/doc/libs/1_64_0/boost/iterator/counting_iterator.hpp
    using iterator_category = std::random_access_iterator_tag;
    using value_type = TNumber;
    using difference_type = TNumber;
    using pointer = TNumber*;
    using reference = const TNumber&;
    using self = CountingIterator<TNumber>;
//    explicit CountingIterator() : m_value{0} {}
    explicit CountingIterator(const TNumber value) : m_value{value} {}
    bool operator==(const self& rhs) const { return m_value == rhs.m_value; }
    bool operator!=(const self& rhs) const { return !(*this == rhs); }
    reference operator*() const { return m_value; }
    difference_type operator-(const self& rhs) const { return m_value - rhs.m_value; }
    /** Prefix, e.g., ++i */
    self& operator++() { ++m_value; return *this; }
    /** Postfix, e.g., i++ */
    self operator++(int) { self x = *this; ++m_value; return x; }
    /** Prefix, e.g., --i */
    self& operator--() { --m_value; return *this; }
    /** Postfix, e.g., i-- */
    self operator--(int) { self x = *this; --m_value; return x; }
    self& operator+=(const difference_type n) { m_value += n; return *this; }
private:
    TNumber m_value;
};

}  // namespace SDV

#endif //SDV_COUNTINGITERATOR_H
