// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMAT_HPP_
#define SFZ_FORMAT_HPP_

#include <sfz/PrintItem.hpp>
#include <sfz/PrintTarget.hpp>

namespace sfz {

template <size_t size> class Format;

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

}  // namespace sfz

#endif  // SFZ_FORMAT_HPP_
