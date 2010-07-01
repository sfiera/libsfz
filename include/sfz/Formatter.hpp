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
};
void print_to(PrintTarget out, const FormattedSint& value);

struct FormattedUint {
    uint64_t value;
    int base;
    size_t min_width;
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

template <typename T> inline typename FormattedInt<T>::Type dec(T value, size_t min_width = 1) {
    typename FormattedInt<T>::Type result = { value, 10, min_width };
    return result;
}

template <typename T> inline typename FormattedInt<T>::Type hex(T value, size_t min_width = 1) {
    typename FormattedInt<T>::Type result = { value, 16, min_width };
    return result;
}

template <typename T> inline typename FormattedInt<T>::Type oct(T value, size_t min_width = 1) {
    typename FormattedInt<T>::Type result = { value, 8, min_width };
    return result;
}

template <typename T> inline typename FormattedInt<T>::Type bin(T value, size_t min_width = 1) {
    typename FormattedInt<T>::Type result = { value, 2, min_width };
    return result;
}

struct EscapedString {
    const StringPiece& string;
};
void print_to(PrintTarget out, const EscapedString& value);

inline EscapedString escape(const StringPiece& string) {
    EscapedString result = { string };
    return result;
}

struct QuotedString {
    const StringPiece& string;
};
void print_to(PrintTarget out, const QuotedString& value);

inline QuotedString quote(const StringPiece& string) {
    QuotedString result = { string };
    return result;
}

}  // namespace sfz

#endif  // SFZ_FORMATTER_HPP_
