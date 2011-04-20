// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FOREACH_HPP_
#define SFZ_FOREACH_HPP_

#include <sfz/algorithm.hpp>
#include <sfz/macros.hpp>

#   if (_MSC_VER >= 1600) && (_MSC_VER < 1700)
#       define SFZ_DECLTYPE(x) decltype(x)
#   elif __GNUC__ == 4
#       define SFZ_DECLTYPE(x) __typeof__(x)
#   else
#       define SFZ_DECLTYPE(x) int
#       error "Don't know how to get decltype-like functionality"
#   endif

#define SFZ_FOREACH(DECLARATION, CONTAINER, BLOCK) \
    do { \
        const SFZ_DECLTYPE(CONTAINER)& __container = (CONTAINER); \
        for (SFZ_DECLTYPE(::sfz::begin(__container)) __begin = ::sfz::begin(__container), \
                __end = ::sfz::end(__container); __begin != __end; ++__begin) { \
            DECLARATION __attribute__((unused)) = *__begin; \
            { BLOCK } \
        } \
    } while (false)

namespace sfz {

template <typename T> class Range;

template <typename T>
Range<T> range(T begin, T end);

template <typename T>
Range<T> range(T end);

// Implementation details follow.

template <typename T>
typename T::const_iterator begin(const T& container) {
    return container.begin();
}

template <typename T, int size>
const T* begin(const T (&array)[size]) {
    return array;
}

template <typename T>
typename T::const_iterator end(const T& container) {
    return container.end();
}

template <typename T, int size>
const T* end(const T (&array)[size]) {
    return array + size;
}

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

#endif  // SFZ_FOREACH_HPP_
