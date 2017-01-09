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

namespace {

void span_complement(
        const StringSlice& input, const StringSlice& chars, StringSlice* span,
        StringSlice* remainder) {
    for (int i : range(input.size())) {
        if (chars.find(input.at(i)) != String::npos) {
            *span      = input.slice(0, i);
            *remainder = input.slice(i);
            return;
        }
    }
    *span      = input;
    *remainder = StringSlice();
}

}  // namespace

void print_format_to(
        PrintTarget out, const char* format_string, const PrintItem* const* items, size_t size) {
    static const StringSlice kBraces     = "{}";
    static const StringSlice kCloseBrace = "}";

    StringSlice f = format_string;

    while (f.size() > 0) {
        StringSlice span;
        StringSlice remainder;
        span_complement(f, kBraces, &span, &remainder);
        if (remainder.size() == 1) {
            span      = f;
            remainder = StringSlice();
        }
        out.push(span);
        f = remainder;
        if (f.size() > 0) {
            if (f.at(0) == f.at(1)) {
                out.push(1, f.at(0));
                f = f.slice(2);
            } else if (f.at(0) == '}') {
                out.push(1, '}');
                f = f.slice(1);
            } else {
                f = f.slice(1);
                span_complement(f, kCloseBrace, &span, &remainder);

                uint32_t index;
                if (string_to_int(span, index)) {
                    if (index < size) {
                        items[index]->print_to(out);
                    }
                }

                if (remainder.empty()) {
                    f = remainder;
                } else {
                    f = remainder.slice(1);
                }
            }
        }
    }
}

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
    print(out, ascii::decode(BytesSlice(data, size)));
}

namespace {

// Maps control characters to their octal-escaped equivalents.  There are two things of note here:
// first, we always specify all three octal digits.  This is to prevent cases where a character
// following the escaped character could be interpreted as part of the escape sequence.  For
// example, we always escape the pair of runes ('\0', '1') as '\0001'.  Second, we use the octal
// equivalents of '\a', '\b', and '\v' rather than the symbolic names.  These characters are rare
// enough that it is considered more informative to provide the numeric name.
const char kEscaped[' '][5] = {
        "\\000", "\\001", "\\002", "\\003", "\\004", "\\005", "\\006", "\\007",
        "\\010", "\\t",   "\\n",   "\\013", "\\014", "\\r",   "\\016", "\\017",
        "\\020", "\\021", "\\022", "\\023", "\\024", "\\025", "\\026", "\\027",
        "\\030", "\\031", "\\032", "\\033", "\\034", "\\035", "\\036", "\\037",
};

}  // namespace

void print_to(PrintTarget out, const EscapedString& value) {
    for (Rune rune : value.string) {
        if (rune < ' ') {
            out.push(kEscaped[rune]);
        } else if (rune == '\'' || rune == '\"' || rune == '\\') {
            out.push(1, '\\');
            out.push(1, rune);
        } else {
            out.push(1, rune);
        }
    }
}

void print_to(PrintTarget out, const QuotedString& value) {
    out.push(1, '"');
    EscapedString item = {value.string};
    print_to(out, item);
    out.push(1, '"');
}

}  // namespace sfz
