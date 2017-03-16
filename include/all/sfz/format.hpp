// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMAT_HPP_
#define SFZ_FORMAT_HPP_

#include <stdint.h>
#include <sfz/print.hpp>
#include <sfz/string.hpp>

namespace sfz {

struct EscapedString;
struct FormattedInt;
class Integer;
struct QuotedString;

FormattedInt dec(Integer value, size_t min_width = 1);
FormattedInt hex(Integer value, size_t min_width = 1);
FormattedInt oct(Integer value, size_t min_width = 1);
FormattedInt bin(Integer value, size_t min_width = 1);

EscapedString escape(const StringSlice& string);

QuotedString quote(const StringSlice& string);

// Implementation details follow.

class Integer {
  public:
    Integer(signed char value);
    Integer(signed short value);
    Integer(signed int value);
    Integer(signed long value);
    Integer(signed long long value);

    Integer(unsigned char value);
    Integer(unsigned short value);
    Integer(unsigned int value);
    Integer(unsigned long value);
    Integer(unsigned long long value);

    bool     negative() const { return _negative; }
    uint64_t abs() const { return _abs; }

  private:
    const bool     _negative;
    const uint64_t _abs;
};

struct FormattedInt {
    Integer value;
    int     base;
    size_t  min_width;
};
void print_to(PrintTarget out, const FormattedInt& value);

inline FormattedInt dec(Integer value, size_t min_width) {
    FormattedInt result = {value, 10, min_width};
    return result;
}

inline FormattedInt hex(Integer value, size_t min_width) {
    FormattedInt result = {value, 16, min_width};
    return result;
}

inline FormattedInt oct(Integer value, size_t min_width) {
    FormattedInt result = {value, 8, min_width};
    return result;
}

inline FormattedInt bin(Integer value, size_t min_width) {
    FormattedInt result = {value, 2, min_width};
    return result;
}

struct EscapedString {
    const StringSlice& string;
};
void print_to(PrintTarget out, const EscapedString& value);

inline EscapedString escape(const StringSlice& string) {
    EscapedString result = {string};
    return result;
}

struct QuotedString {
    const StringSlice& string;
};
void print_to(PrintTarget out, const QuotedString& value);

inline QuotedString quote(const StringSlice& string) {
    QuotedString result = {string};
    return result;
}

}  // namespace sfz

#endif  // SFZ_FORMAT_HPP_
