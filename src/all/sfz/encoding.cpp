// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/encoding.hpp"

#include <algorithm>
#include "sfz/bytes.hpp"
#include "sfz/foreach.hpp"
#include "sfz/string.hpp"

namespace sfz {

namespace {

// Identifies surrogate code points.
//
// UTF-16 represents code points outside the basic multilingual plane (plane 0) with a pair of
// plane 0 code points, the first of which is in the range U+D800 to U+DBFF, and the second of which
// is in the range U+DC00 to U+DFFF.  These are not valid code points within a String, so we need
// to identify them as invalid.
//
// @param [in] code     A code point to test.
// @returns             true iff `code` is a surrogate code.
inline bool is_surrogate(Rune rune) {
    return (rune & 0xfffff800) == 0x00d800;
}

}  // namespace

const Rune kUnknownCodePoint = 0x00fffd;  // REPLACEMENT CHARACTER.
const Rune kAsciiUnknownCodePoint = 0x00003f;  // QUESTION MARK.

bool is_valid_code_point(Rune rune) {
    return (rune <= 0x10ffff) && (!is_surrogate(rune));
}

namespace ascii {

void Ascii::encode_to(WriteTarget out, const StringSlice& string) {
    foreach (Rune r, string) {
        if (r > 0x7f) {
            out.push(1, kAsciiUnknownCodePoint);
        } else {
            out.push(1, r);
        }
    }
}

void Ascii::decode_to(PrintTarget out, const BytesSlice& bytes) {
    foreach (uint8_t byte, bytes) {
        if (byte & 0x80) {
            out.push(1, kUnknownCodePoint);
        } else {
            out.push(1, byte);
        }
    }
}

}  // namespace ascii

namespace latin1 {

void Latin1::encode_to(WriteTarget out, const StringSlice& string) {
    foreach (Rune r, string) {
        if (r > 0xff) {
            out.push(1, kAsciiUnknownCodePoint);
        } else {
            out.push(1, r);
        }
    }
}

void Latin1::decode_to(PrintTarget out, const BytesSlice& bytes) {
    foreach (uint8_t byte, bytes) {
        out.push(1, byte);
    }
}

}  // namespace latin1

namespace utf8 {

namespace {

// Returns the left-most or right-most `bits` bits of `byte`.
//
// @param [in] bits     The number of bits.  Only meaningful within [1, 7].
// @param [in] byte     The byte to pull the bits from.
inline uint8_t left(int bits, uint8_t byte) { return byte & ~((1 << (8 - bits)) - 1); }
inline uint8_t right(int bits, uint8_t byte) { return byte & ((1 << bits) - 1); }

// Returns true iff `byte` encodes an ASCII byte.
bool is_ascii(uint8_t byte) {
    return left(1, byte) == 0;
}

// Returns true iff `byte` is a UTF-8 multibyte character head of size `count`.
//
// @param [in] count    The size of the multibyte character to consider.  Must lie within [2, 4].
// @param [in] byte     The byte to test.
inline bool is_multibyte_head(int count, uint8_t byte) {
    return left(count + 1, byte) == left(count, 0xff);
}

// Returns true iff `byte` is a UTF-8 continuation character.
//
// @param [in] byte     The byte to test.
inline bool is_continuation_byte(uint8_t byte) {
    return left(2, byte) == left(1, 0xff);
}

// Thresholds for UTF-8 multibyte encoding.  A single byte can represent code points in the range
// [0, k[0]), a two-byte character can represent code points in the range [k[0], k[1]), a
// three-byte character [k[1], k[2]), and a four-byte character [k[2], 0x110000).
const Rune kUtf8Max[4] = { 0x80, 0x800, 0x10000 };

}  // namespace

void Utf8::encode_to(WriteTarget out, const StringSlice& string) {
    foreach (Rune rune, string) {
        if (rune < kUtf8Max[0]) {
            out.push(1, rune);
        } else if (rune < kUtf8Max[1]) {
            out.push(1, left(2, 0xff) | (rune >> 6));
            out.push(1, left(1, 0xff) | right(6, rune >> 0));
        } else if (rune < kUtf8Max[2]) {
            out.push(1, left(3, 0xff) | (rune >> 12));
            out.push(1, left(1, 0xff) | right(6, rune >> 6));
            out.push(1, left(1, 0xff) | right(6, rune >> 0));
        } else {
            out.push(1, left(4, 0xff) | (rune >> 18));
            out.push(1, left(1, 0xff) | right(6, rune >> 12));
            out.push(1, left(1, 0xff) | right(6, rune >> 6));
            out.push(1, left(1, 0xff) | right(6, rune >> 0));
        }
    }
}

void Utf8::decode_to(PrintTarget out, const BytesSlice& bytes) {
    int multibytes_expected = 0;
    int multibytes_seen = 0;
    Rune rune = 0;

    foreach (uint8_t byte, bytes) {
        if (multibytes_expected > 0) {
            if (is_continuation_byte(byte)) {
                rune = (rune << 6) | right(6, byte);
                --multibytes_expected;
                ++multibytes_seen;
                if (multibytes_expected == 0) {
                    if (is_valid_code_point(rune) && (rune >= kUtf8Max[multibytes_seen - 2])) {
                        out.push(1, rune);
                    } else {
                        out.push(multibytes_seen, kUnknownCodePoint);
                    }
                }
                goto next_byte;
            } else {
                multibytes_expected = 0;
                out.push(multibytes_seen, kUnknownCodePoint);
            }
        }

        if (is_ascii(byte)) {
            out.push(1, byte);
            goto next_byte;
        }
        for (int count = 2; count <= 4; ++count) {
            if (is_multibyte_head(count, byte)) {
                multibytes_expected = count - 1;
                multibytes_seen = 1;
                rune = right(7 - count, byte);
                goto next_byte;
            }
        }
        out.push(1, kUnknownCodePoint);
next_byte:
        continue;
    }

    if (multibytes_expected) {
        out.push(multibytes_seen, kUnknownCodePoint);
    }
}

}  // namespace utf8

namespace macroman {

namespace {

uint16_t kMacRomanSupplement[0x80] = {
    0x00C4,  // LATIN CAPITAL LETTER A WITH DIAERESIS
    0x00C5,  // LATIN CAPITAL LETTER A WITH RING ABOVE
    0x00C7,  // LATIN CAPITAL LETTER C WITH CEDILLA
    0x00C9,  // LATIN CAPITAL LETTER E WITH ACUTE
    0x00D1,  // LATIN CAPITAL LETTER N WITH TILDE
    0x00D6,  // LATIN CAPITAL LETTER O WITH DIAERESIS
    0x00DC,  // LATIN CAPITAL LETTER U WITH DIAERESIS
    0x00E1,  // LATIN SMALL LETTER A WITH ACUTE
    0x00E0,  // LATIN SMALL LETTER A WITH GRAVE
    0x00E2,  // LATIN SMALL LETTER A WITH CIRCUMFLEX
    0x00E4,  // LATIN SMALL LETTER A WITH DIAERESIS
    0x00E3,  // LATIN SMALL LETTER A WITH TILDE
    0x00E5,  // LATIN SMALL LETTER A WITH RING ABOVE
    0x00E7,  // LATIN SMALL LETTER C WITH CEDILLA
    0x00E9,  // LATIN SMALL LETTER E WITH ACUTE
    0x00E8,  // LATIN SMALL LETTER E WITH GRAVE
    0x00EA,  // LATIN SMALL LETTER E WITH CIRCUMFLEX
    0x00EB,  // LATIN SMALL LETTER E WITH DIAERESIS
    0x00ED,  // LATIN SMALL LETTER I WITH ACUTE
    0x00EC,  // LATIN SMALL LETTER I WITH GRAVE
    0x00EE,  // LATIN SMALL LETTER I WITH CIRCUMFLEX
    0x00EF,  // LATIN SMALL LETTER I WITH DIAERESIS
    0x00F1,  // LATIN SMALL LETTER N WITH TILDE
    0x00F3,  // LATIN SMALL LETTER O WITH ACUTE
    0x00F2,  // LATIN SMALL LETTER O WITH GRAVE
    0x00F4,  // LATIN SMALL LETTER O WITH CIRCUMFLEX
    0x00F6,  // LATIN SMALL LETTER O WITH DIAERESIS
    0x00F5,  // LATIN SMALL LETTER O WITH TILDE
    0x00FA,  // LATIN SMALL LETTER U WITH ACUTE
    0x00F9,  // LATIN SMALL LETTER U WITH GRAVE
    0x00FB,  // LATIN SMALL LETTER U WITH CIRCUMFLEX
    0x00FC,  // LATIN SMALL LETTER U WITH DIAERESIS
    0x2020,  // DAGGER
    0x00B0,  // DEGREE SIGN
    0x00A2,  // CENT SIGN
    0x00A3,  // POUND SIGN
    0x00A7,  // SECTION SIGN
    0x2022,  // BULLET
    0x00B6,  // PILCROW SIGN
    0x00DF,  // LATIN SMALL LETTER SHARP S
    0x00AE,  // REGISTERED SIGN
    0x00A9,  // COPYRIGHT SIGN
    0x2122,  // TRADE MARK SIGN
    0x00B4,  // ACUTE ACCENT
    0x00A8,  // DIAERESIS
    0x2260,  // NOT EQUAL TO
    0x00C6,  // LATIN CAPITAL LETTER AE
    0x00D8,  // LATIN CAPITAL LETTER O WITH STROKE
    0x221E,  // INFINITY
    0x00B1,  // PLUS-MINUS SIGN
    0x2264,  // LESS-THAN OR EQUAL TO
    0x2265,  // GREATER-THAN OR EQUAL TO
    0x00A5,  // YEN SIGN
    0x00B5,  // MICRO SIGN
    0x2202,  // PARTIAL DIFFERENTIAL
    0x2211,  // N-ARY SUMMATION
    0x220F,  // N-ARY PRODUCT
    0x03C0,  // GREEK SMALL LETTER PI
    0x222B,  // INTEGRAL
    0x00AA,  // FEMININE ORDINAL INDICATOR
    0x00BA,  // MASCULINE ORDINAL INDICATOR
    0x03A9,  // GREEK CAPITAL LETTER OMEGA
    0x00E6,  // LATIN SMALL LETTER AE
    0x00F8,  // LATIN SMALL LETTER O WITH STROKE
    0x00BF,  // INVERTED QUESTION MARK
    0x00A1,  // INVERTED EXCLAMATION MARK
    0x00AC,  // NOT SIGN
    0x221A,  // SQUARE ROOT
    0x0192,  // LATIN SMALL LETTER F WITH HOOK
    0x2248,  // ALMOST EQUAL TO
    0x2206,  // INCREMENT
    0x00AB,  // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    0x00BB,  // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    0x2026,  // HORIZONTAL ELLIPSIS
    0x00A0,  // NO-BREAK SPACE
    0x00C0,  // LATIN CAPITAL LETTER A WITH GRAVE
    0x00C3,  // LATIN CAPITAL LETTER A WITH TILDE
    0x00D5,  // LATIN CAPITAL LETTER O WITH TILDE
    0x0152,  // LATIN CAPITAL LIGATURE OE
    0x0153,  // LATIN SMALL LIGATURE OE
    0x2013,  // EN DASH
    0x2014,  // EM DASH
    0x201C,  // LEFT DOUBLE QUOTATION MARK
    0x201D,  // RIGHT DOUBLE QUOTATION MARK
    0x2018,  // LEFT SINGLE QUOTATION MARK
    0x2019,  // RIGHT SINGLE QUOTATION MARK
    0x00F7,  // DIVISION SIGN
    0x25CA,  // LOZENGE
    0x00FF,  // LATIN SMALL LETTER Y WITH DIAERESIS
    0x0178,  // LATIN CAPITAL LETTER Y WITH DIAERESIS
    0x2044,  // FRACTION SLASH
    0x20AC,  // EURO SIGN
    0x2039,  // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
    0x203A,  // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
    0xFB01,  // LATIN SMALL LIGATURE FI
    0xFB02,  // LATIN SMALL LIGATURE FL
    0x2021,  // DOUBLE DAGGER
    0x00B7,  // MIDDLE DOT
    0x201A,  // SINGLE LOW-9 QUOTATION MARK
    0x201E,  // DOUBLE LOW-9 QUOTATION MARK
    0x2030,  // PER MILLE SIGN
    0x00C2,  // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    0x00CA,  // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    0x00C1,  // LATIN CAPITAL LETTER A WITH ACUTE
    0x00CB,  // LATIN CAPITAL LETTER E WITH DIAERESIS
    0x00C8,  // LATIN CAPITAL LETTER E WITH GRAVE
    0x00CD,  // LATIN CAPITAL LETTER I WITH ACUTE
    0x00CE,  // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    0x00CF,  // LATIN CAPITAL LETTER I WITH DIAERESIS
    0x00CC,  // LATIN CAPITAL LETTER I WITH GRAVE
    0x00D3,  // LATIN CAPITAL LETTER O WITH ACUTE
    0x00D4,  // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    0xF8FF,  // Apple logo
    0x00D2,  // LATIN CAPITAL LETTER O WITH GRAVE
    0x00DA,  // LATIN CAPITAL LETTER U WITH ACUTE
    0x00DB,  // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    0x00D9,  // LATIN CAPITAL LETTER U WITH GRAVE
    0x0131,  // LATIN SMALL LETTER DOTLESS I
    0x02C6,  // MODIFIER LETTER CIRCUMFLEX ACCENT
    0x02DC,  // SMALL TILDE
    0x00AF,  // MACRON
    0x02D8,  // BREVE
    0x02D9,  // DOT ABOVE
    0x02DA,  // RING ABOVE
    0x00B8,  // CEDILLA
    0x02DD,  // DOUBLE ACUTE ACCENT
    0x02DB,  // OGONEK
    0x02C7,  // CARON
};

}  // namespace

void MacRoman::encode_to(WriteTarget out, const StringSlice& string) {
    foreach (Rune r, string) {
        if (r <= 0x7f) {
            out.push(1, r);
        } else {
            foreach (int i, range(0x80)) {
                if (r == kMacRomanSupplement[i]) {
                    out.push(1, 0x80 + i);
                    goto next_rune;
                }
            }
            out.push(1, kAsciiUnknownCodePoint);
        }
next_rune:
        continue;
    }
}

void MacRoman::decode_to(PrintTarget out, const BytesSlice& bytes) {
    foreach (uint8_t byte, bytes) {
        if (byte < 0x80) {
            out.push(1, byte);
        } else {
            out.push(1, kMacRomanSupplement[byte - 0x80]);
        }
    }
}

}  // namespace macroman

}  // namespace sfz
