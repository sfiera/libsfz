// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_RANGE_HPP_
#define SFZ_RANGE_HPP_

#include <sfz/Compare.hpp>

namespace sfz {

template <typename T>
class RangeIterator {
  public:
    typedef T value_type;
    typedef const value_type* pointer;
    typedef const value_type* const_pointer;
    typedef const value_type& reference;
    typedef const value_type& const_reference;

    RangeIterator(value_type value): _value(value) { }

    value_type operator*() const { return _value; }
    pointer operator->() const { return &_value; }

    RangeIterator& operator++() { ++_value; return *this; }
    RangeIterator operator++(int) { return _value++; }
    RangeIterator& operator--() { --_value; return *this; }
    RangeIterator operator--(int) { return _value--; }

  private:
    value_type _value;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
class Range {
  public:
    typedef T value_type;

    typedef RangeIterator<T> iterator;
    typedef RangeIterator<T> const_iterator;

    Range(value_type first, value_type last): _first(first), _last(last) {}

    const_iterator begin() const { return _first; }
    const_iterator end() const { return _last; }

  private:
    iterator _first;
    iterator _last;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
int compare(RangeIterator<T> x, RangeIterator<T> y) {
    return compare(*x, *y);
}

SFZ_OPERATORS_BASED_ON_COMPARE_T(template <typename T>, RangeIterator<T>)

template <typename T>
Range<T> range(T begin, T end) {
    return Range<T>(begin, end);
}

template <typename T>
Range<T> range(T end) {
    return Range<T>(T(), end);
}

}  // namespace sfz

#endif // _SFZ_RANGE_HPP
