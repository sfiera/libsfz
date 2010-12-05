// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_RANGE_HPP_
#define SFZ_RANGE_HPP_

namespace sfz {

template <typename T>
class basic_range {
  public:
    typedef T const_iterator;

    basic_range(T begin, T end) : _begin(begin), _end(end) {}

    const_iterator begin() const { return _begin; }
    const_iterator end() const { return _end; }

  private:
    const_iterator _begin;
    const_iterator _end;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
basic_range<T> range(T begin, T end) {
    return basic_range<T>(begin, end);
}

template <typename T>
basic_range<T> range(T end) {
    return basic_range<T>(T(), end);
}

}  // namespace sfz

#endif // _SFZ_RANGE_HPP
