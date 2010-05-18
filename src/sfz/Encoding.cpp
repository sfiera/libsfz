// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/Encoding.hpp"

#include <algorithm>
#include "sfz/Bytes.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"
#include "sfz/String.hpp"

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

void Ascii::encode_to(WriteTarget out, const StringPiece& string) {
    foreach (it, string) {
        if (*it > 0x7f) {
            out.append(1, kAsciiUnknownCodePoint);
        } else {
            out.append(1, *it);
        }
    }
}

void Ascii::decode_to(PrintTarget out, const BytesPiece& bytes) {
    foreach (it, bytes) {
        uint8_t c = *it;
        if (c & 0x80) {
            out.append(1, kUnknownCodePoint);
        } else {
            out.append(1, c);
        }
    }
}

}  // namespace ascii

namespace latin1 {

void Latin1::encode_to(WriteTarget out, const StringPiece& string) {
    foreach (it, string) {
        if (*it > 0xff) {
            out.append(1, kAsciiUnknownCodePoint);
        } else {
            out.append(1, *it);
        }
    }
}

void Latin1::decode_to(PrintTarget out, const BytesPiece& bytes) {
    foreach (it, bytes) {
        out.append(1, *it);
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

void Utf8::encode_to(WriteTarget out, const StringPiece& string) {
    foreach (it, string) {
        const Rune rune = *it;
        if (rune < kUtf8Max[0]) {
            out.append(1, rune);
        } else if (rune < kUtf8Max[1]) {
            out.append(1, left(2, 0xff) | (rune >> 6));
            out.append(1, left(1, 0xff) | right(6, rune >> 0));
        } else if (rune < kUtf8Max[2]) {
            out.append(1, left(3, 0xff) | (rune >> 12));
            out.append(1, left(1, 0xff) | right(6, rune >> 6));
            out.append(1, left(1, 0xff) | right(6, rune >> 0));
        } else {
            out.append(1, left(4, 0xff) | (rune >> 18));
            out.append(1, left(1, 0xff) | right(6, rune >> 12));
            out.append(1, left(1, 0xff) | right(6, rune >> 6));
            out.append(1, left(1, 0xff) | right(6, rune >> 0));
        }
    }
}

void Utf8::decode_to(PrintTarget out, const BytesPiece& bytes) {
    int multibytes_expected = 0;
    int multibytes_seen = 0;
    Rune rune = 0;

    foreach (it, bytes) {
        if (multibytes_expected > 0) {
            if (is_continuation_byte(*it)) {
                rune = (rune << 6) | right(6, *it);
                --multibytes_expected;
                ++multibytes_seen;
                if (multibytes_expected == 0) {
                    if (is_valid_code_point(rune) && (rune >= kUtf8Max[multibytes_seen - 2])) {
                        out.append(1, rune);
                    } else {
                        out.append(multibytes_seen, kUnknownCodePoint);
                    }
                }
                goto next_byte;
            } else {
                multibytes_expected = 0;
                out.append(multibytes_seen, kUnknownCodePoint);
            }
        }

        if (is_ascii(*it)) {
            out.append(1, *it);
            goto next_byte;
        }
        for (int count = 2; count <= 4; ++count) {
            if (is_multibyte_head(count, *it)) {
                multibytes_expected = count - 1;
                multibytes_seen = 1;
                rune = right(7 - count, *it);
                goto next_byte;
            }
        }
        out.append(1, kUnknownCodePoint);
next_byte:
        continue;
    }

    if (multibytes_expected) {
        out.append(multibytes_seen, kUnknownCodePoint);
    }
}

}  // namespace utf8

}  // namespace sfz
