// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Formatter.hpp"

#include "sfz/Foreach.hpp"

namespace sfz {

EscapedString escape(const StringPiece& string) { return EscapedString(string); }
QuotedString quote(const StringPiece& string) { return QuotedString(string); }

void print_to(PrintTarget out, const FormattedSint& value) {
    if (value.value < 0) {
        out.append(1, '-');
        print_to(out, FormattedUint(-value.value, value.base, value.min_width));
    } else {
        print_to(out, FormattedUint(value.value, value.base, value.min_width));
    }
}

void print_to(PrintTarget out, const FormattedUint& value) {
    static const char kDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    uint8_t buffer[64];
    size_t size = 0;
    uint8_t* data = buffer + 64;
    uint64_t v = value.value;

    while (v > 0) {
        --data;
        ++size;
        *data = kDigits[v % value.base];
        v /= value.base;
    }

    if (size < value.min_width) {
        out.append(value.min_width - size, '0');
    }
    out.append(BytesPiece(data, size), ascii_encoding());
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
    foreach (it, value.string) {
        Rune rune = *it;
        if (rune < ' ') {
            out.append(kEscaped[rune]);
        } else if (rune == '\'' || rune == '\"' || rune == '\\') {
            out.append(1, '\\');
            out.append(1, rune);
        } else {
            out.append(1, rune);
        }
    }
}

void print_to(PrintTarget out, const QuotedString& value) {
    out.append(1, '"');
    print_to(out, EscapedString(value.string));
    out.append(1, '"');
}

}  // namespace sfz
