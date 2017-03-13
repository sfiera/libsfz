// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/format.hpp>

#include <sfz/bytes.hpp>
#include <sfz/encoding.hpp>
#include <sfz/range.hpp>
#include <sfz/string-utils.hpp>
#include <sfz/string.hpp>

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

void print_to(PrintTarget out, const FormattedInt& value) {
    static const char kDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    uint8_t  buffer[64];
    size_t   size = 0;
    uint8_t* data = buffer + 64;
    uint64_t v    = value.value.abs();

    while (v > 0) {
        --data;
        ++size;
        *data = kDigits[v % value.base];
        v /= value.base;
    }

    if (value.value.negative()) {
        out.push(1, '-');
    }
    if (size < value.min_width) {
        out.push(value.min_width - size, '0');
    }
    print(out, utf8::decode(BytesSlice(data, size)));
}

}  // namespace sfz
