// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMATTER_HPP_
#define SFZ_FORMATTER_HPP_

#include "sfz/PrintTarget.hpp"

#include "sfz/String.hpp"

namespace sfz {

template <typename T> struct FormattedInt;
template <typename T> typename FormattedInt<T>::Type dec(T value, size_t min_width = 1);
template <typename T> typename FormattedInt<T>::Type hex(T value, size_t min_width = 1);
template <typename T> typename FormattedInt<T>::Type oct(T value, size_t min_width = 1);
template <typename T> typename FormattedInt<T>::Type bin(T value, size_t min_width = 1);

struct EscapedString;
EscapedString escape(const StringPiece& string);

struct QuotedString;
QuotedString quote(const StringPiece& string);

struct FormattedSint {
    int64_t value;
    int base;
    size_t min_width;
    FormattedSint(int64_t value, int base, size_t min_width)
        : value(value), base(base), min_width(min_width) { }
};
void print_to(PrintTarget out, const FormattedSint& value);

struct FormattedUint {
    uint64_t value;
    int base;
    size_t min_width;
    FormattedUint(uint64_t value, int base, size_t min_width)
        : value(value), base(base), min_width(min_width) { }
};
void print_to(PrintTarget out, const FormattedUint& value);

template <> struct FormattedInt<signed char> { typedef FormattedSint Type; };
template <> struct FormattedInt<signed short> { typedef FormattedSint Type; };
template <> struct FormattedInt<signed int> { typedef FormattedSint Type; };
template <> struct FormattedInt<signed long> { typedef FormattedSint Type; };
template <> struct FormattedInt<signed long long> { typedef FormattedSint Type; };
template <> struct FormattedInt<unsigned char> { typedef FormattedUint Type; };
template <> struct FormattedInt<unsigned short> { typedef FormattedUint Type; };
template <> struct FormattedInt<unsigned int> { typedef FormattedUint Type; };
template <> struct FormattedInt<unsigned long> { typedef FormattedUint Type; };
template <> struct FormattedInt<unsigned long long> { typedef FormattedUint Type; };

template <typename T> typename FormattedInt<T>::Type dec(T value, size_t min_width = 1) {
    return typename FormattedInt<T>::Type(value, 10, min_width);
}

template <typename T> typename FormattedInt<T>::Type hex(T value, size_t min_width = 1) {
    return typename FormattedInt<T>::Type(value, 16, min_width);
}

template <typename T> typename FormattedInt<T>::Type oct(T value, size_t min_width = 1) {
    return typename FormattedInt<T>::Type(value, 8, min_width);
}

template <typename T> typename FormattedInt<T>::Type bin(T value, size_t min_width = 1) {
    return typename FormattedInt<T>::Type(value, 2, min_width);
}

struct EscapedString {
    const StringPiece& string;
    EscapedString(const StringPiece& s) : string(s) { }
};
void print_to(PrintTarget out, const EscapedString& value);

struct QuotedString {
    const StringPiece& string;
    QuotedString(const StringPiece& s) : string(s) { }
};
void print_to(PrintTarget out, const QuotedString& value);

}  // namespace sfz

#endif  // SFZ_FORMATTER_HPP_
