// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/format.hpp>

#include <pn/string>
#include <sfz/encoding.hpp>
#include <sfz/range.hpp>
#include <sfz/string-utils.hpp>

namespace sfz {

Integer::Integer(signed char value)
        : _negative(value < 0), _abs(_negative ? -static_cast<int64_t>(value) : value) {}

Integer::Integer(signed short value)
        : _negative(value < 0), _abs(_negative ? -static_cast<int64_t>(value) : value) {}

Integer::Integer(signed int value)
        : _negative(value < 0), _abs(_negative ? -static_cast<int64_t>(value) : value) {}

Integer::Integer(signed long value)
        : _negative(value < 0), _abs(_negative ? -static_cast<int64_t>(value) : value) {}

Integer::Integer(signed long long value)
        : _negative(value < 0), _abs(_negative ? -static_cast<int64_t>(value) : value) {}

Integer::Integer(unsigned char value) : _negative(false), _abs(value) {}

Integer::Integer(unsigned short value) : _negative(false), _abs(value) {}

Integer::Integer(unsigned int value) : _negative(false), _abs(value) {}

Integer::Integer(unsigned long value) : _negative(false), _abs(value) {}

Integer::Integer(unsigned long long value) : _negative(false), _abs(value) {}

static pn::string int_to_string(Integer i, int base, int min_width) {
    static const char kDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    char     buffer[64];
    int      size = 0;
    char*    data = buffer + 64;
    uint64_t v    = i.abs();

    while (v > 0) {
        --data;
        ++size;
        *data = kDigits[v % base];
        v /= base;
    }

    pn::string result;
    if (i.negative()) {
        result += pn::rune{'-'};
    }
    if (size < min_width) {
        for (int i = size; i < min_width; ++i) {
            result += pn::rune{'0'};
        }
    }
    result += pn::string_view{data, size};
    return result;
}

pn::string dec(Integer value, int min_width) { return int_to_string(value, 10, min_width); }
pn::string hex(Integer value, int min_width) { return int_to_string(value, 16, min_width); }
pn::string oct(Integer value, int min_width) { return int_to_string(value, 8, min_width); }
pn::string bin(Integer value, int min_width) { return int_to_string(value, 2, min_width); }

}  // namespace sfz
