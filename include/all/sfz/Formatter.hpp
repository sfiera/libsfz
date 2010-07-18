// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMATTER_HPP_
#define SFZ_FORMATTER_HPP_

#include <sfz/Integer.hpp>
#include <sfz/PrintTarget.hpp>
#include <sfz/String.hpp>

namespace sfz {

struct FormattedInt;
FormattedInt dec(Integer value, size_t min_width = 1);
FormattedInt hex(Integer value, size_t min_width = 1);
FormattedInt oct(Integer value, size_t min_width = 1);
FormattedInt bin(Integer value, size_t min_width = 1);

struct EscapedString;
EscapedString escape(const StringPiece& string);

struct QuotedString;
QuotedString quote(const StringPiece& string);

struct FormattedInt {
    Integer value;
    int base;
    size_t min_width;
};
void print_to(PrintTarget out, const FormattedInt& value);

inline FormattedInt dec(Integer value, size_t min_width) {
    FormattedInt result = { value, 10, min_width };
    return result;
}

inline FormattedInt hex(Integer value, size_t min_width) {
    FormattedInt result = { value, 16, min_width };
    return result;
}

inline FormattedInt oct(Integer value, size_t min_width) {
    FormattedInt result = { value, 8, min_width };
    return result;
}

inline FormattedInt bin(Integer value, size_t min_width) {
    FormattedInt result = { value, 2, min_width };
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
