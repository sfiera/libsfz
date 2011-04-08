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
template <size_t size> class Format;
struct FormattedInt;
class Integer;
struct QuotedString;

Format<16> format(
        const char* format_string,
        const PrintItem& item0 = PrintItem(), const PrintItem& item1 = PrintItem(),
        const PrintItem& item2 = PrintItem(), const PrintItem& item3 = PrintItem(),
        const PrintItem& item4 = PrintItem(), const PrintItem& item5 = PrintItem(),
        const PrintItem& item6 = PrintItem(), const PrintItem& item7 = PrintItem(),
        const PrintItem& item8 = PrintItem(), const PrintItem& item9 = PrintItem(),
        const PrintItem& item10 = PrintItem(), const PrintItem& item11 = PrintItem(),
        const PrintItem& item12 = PrintItem(), const PrintItem& item13 = PrintItem(),
        const PrintItem& item14 = PrintItem(), const PrintItem& item15 = PrintItem());

FormattedInt dec(Integer value, size_t min_width = 1);
FormattedInt hex(Integer value, size_t min_width = 1);
FormattedInt oct(Integer value, size_t min_width = 1);
FormattedInt bin(Integer value, size_t min_width = 1);

EscapedString escape(const StringSlice& string);

QuotedString quote(const StringSlice& string);

// Implementation details follow.

template <size_t size>
struct Format {
    const char* format_string;
    const PrintItem* items[size];
};

void print_format_to(
        PrintTarget out, const char* format_string, const PrintItem* const* items, size_t size);

template <size_t size>
inline void print_to(PrintTarget out, const Format<size>& format) {
    print_format_to(out, format.format_string, format.items, size);
}

inline Format<16> format(
        const char* format_string,
        const PrintItem& item0, const PrintItem& item1,
        const PrintItem& item2, const PrintItem& item3,
        const PrintItem& item4, const PrintItem& item5,
        const PrintItem& item6, const PrintItem& item7,
        const PrintItem& item8, const PrintItem& item9,
        const PrintItem& item10, const PrintItem& item11,
        const PrintItem& item12, const PrintItem& item13,
        const PrintItem& item14, const PrintItem& item15) {
    Format<16> result = {
        format_string,
        {
            &item0, &item1, &item2, &item3, &item4, &item5, &item6, &item7,
            &item8, &item9, &item10, &item11, &item12, &item13, &item14, &item15,
        },
    };
    return result;
}

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

    bool negative() const { return _negative; }
    uint64_t abs() const { return _abs; }

  private:
    const bool _negative;
    const uint64_t _abs;
};

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
    const StringSlice& string;
};
void print_to(PrintTarget out, const EscapedString& value);

inline EscapedString escape(const StringSlice& string) {
    EscapedString result = { string };
    return result;
}

struct QuotedString {
    const StringSlice& string;
};
void print_to(PrintTarget out, const QuotedString& value);

inline QuotedString quote(const StringSlice& string) {
    QuotedString result = { string };
    return result;
}

}  // namespace sfz

#endif  // SFZ_FORMAT_HPP_
