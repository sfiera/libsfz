// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Formatter.hpp"

#include "sfz/Bytes.hpp"
#include "sfz/encoding.hpp"
#include "sfz/foreach.hpp"

namespace sfz {

void print_to(PrintTarget out, const FormattedInt& value) {
    static const char kDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    uint8_t buffer[64];
    size_t size = 0;
    uint8_t* data = buffer + 64;
    uint64_t v = value.value.abs();

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
    foreach (Rune rune, value.string) {
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
    EscapedString item = { value.string };
    print_to(out, item);
    out.push(1, '"');
}

}  // namespace sfz
