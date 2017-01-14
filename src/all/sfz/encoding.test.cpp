// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/encoding.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/bytes.hpp>
#include <sfz/range.hpp>
#include <sfz/string.hpp>

using testing::Eq;
using testing::Test;

namespace sfz {
namespace {

typedef Test EncodingTest;

TEST_F(EncodingTest, IsValidCodePoint) {
    // Basic multilingual plane, before the surrogate code points (valid).
    EXPECT_THAT(is_valid_code_point(0x0000), true);
    EXPECT_THAT(is_valid_code_point(0x0001), true);
    EXPECT_THAT(is_valid_code_point(0x007f), true);
    EXPECT_THAT(is_valid_code_point(0x0080), true);
    EXPECT_THAT(is_valid_code_point(0x0100), true);
    EXPECT_THAT(is_valid_code_point(0x1000), true);
    EXPECT_THAT(is_valid_code_point(0xd7ff), true);

    // Surrogate code points (invalid).
    EXPECT_THAT(is_valid_code_point(0xd800), false);
    EXPECT_THAT(is_valid_code_point(0xdada), false);
    EXPECT_THAT(is_valid_code_point(0xdc00), false);
    EXPECT_THAT(is_valid_code_point(0xdfff), false);

    // The remainder of the basic multilingual plane (valid).
    EXPECT_THAT(is_valid_code_point(0xe000), true);
    EXPECT_THAT(is_valid_code_point(0xf000), true);
    EXPECT_THAT(is_valid_code_point(0xffff), true);

    // Code points in higher planes (valid).
    EXPECT_THAT(is_valid_code_point(0x010000), true);
    EXPECT_THAT(is_valid_code_point(0x0fffff), true);
    EXPECT_THAT(is_valid_code_point(0x100000), true);
    EXPECT_THAT(is_valid_code_point(0x10ffff), true);

    // Code points higher than plane 16 (invalid).
    EXPECT_THAT(is_valid_code_point(0x00110000), false);
    EXPECT_THAT(is_valid_code_point(0x01000000), false);
    EXPECT_THAT(is_valid_code_point(0x10000000), false);
    EXPECT_THAT(is_valid_code_point(0xffffffff), false);
}

typedef Test AsciiEncodingTest;

TEST_F(AsciiEncodingTest, DecodeValid) {
    Bytes bytes;
    for (int i : range(0x80)) {
        bytes.append(1, i);
    }

    String string(ascii::decode(bytes));
    ASSERT_THAT(string.size(), Eq(bytes.size()));
    for (int i : range(string.size())) {
        EXPECT_THAT(string.at(i), Eq<Rune>(i));
    }
}

TEST_F(AsciiEncodingTest, EncodeValid) {
    String string;
    for (int i : range(0x80)) {
        string.append(1, i);
    }

    Bytes bytes(ascii::encode(string));
    ASSERT_THAT(bytes.size(), Eq(string.size()));
    for (size_t i : range(bytes.size())) {
        EXPECT_THAT(bytes.at(i), Eq<uint8_t>(i));
    }
}

TEST_F(AsciiEncodingTest, DecodeInvalid) {
    Bytes bytes;
    for (int i : range(0x80, 0x100)) {
        bytes.append(1, i);
    }

    String string(ascii::decode(bytes));
    ASSERT_THAT(string.size(), Eq(bytes.size()));
    for (int i : range(string.size())) {
        EXPECT_THAT(string.at(i), Eq(kUnknownCodePoint));
    }
}

TEST_F(AsciiEncodingTest, EncodeInvalid) {
    String string;
    for (int i : range(0x8, 0x100)) {
        string.append(1, i * 0x10);
    }

    Bytes bytes(ascii::encode(string));
    ASSERT_THAT(bytes.size(), Eq(string.size()));
    for (int i : range(bytes.size())) {
        EXPECT_THAT(bytes.at(i), Eq(kAsciiUnknownCodePoint));
    }
}

typedef Test Latin1EncodingTest;

TEST_F(Latin1EncodingTest, Decode) {
    Bytes bytes;
    for (int i : range(0x100)) {
        bytes.append(1, i);
    }

    String string(latin1::decode(bytes));
    ASSERT_THAT(string.size(), Eq(bytes.size()));
    for (int i : range(string.size())) {
        EXPECT_THAT(string.at(i), Eq<Rune>(i));
    }
}

TEST_F(Latin1EncodingTest, EncodeValid) {
    String string;
    for (int i : range(0x100)) {
        string.append(1, i);
    }

    Bytes bytes(latin1::encode(string));
    ASSERT_THAT(bytes.size(), Eq(string.size()));
    for (int i : range(bytes.size())) {
        EXPECT_THAT(bytes.at(i), Eq<uint8_t>(i));
    }
}

TEST_F(Latin1EncodingTest, EncodeInvalid) {
    String string;
    for (int i : range(0x1, 0x100)) {
        // Some of these generated code points will be in the surrogate code point range, meaning
        // they cannot be appended to the string.  Append kUnknownCodePoint instead, which is
        // valid, but unencodable, so it will end up being encoded as kAsciiUnknownCodePoint.
        if (is_valid_code_point(i * 0x100)) {
            string.append(1, i * 0x100);
        } else {
            string.append(1, kUnknownCodePoint);
        }
    }

    Bytes bytes(latin1::encode(string));
    ASSERT_THAT(bytes.size(), Eq(string.size()));
    for (int i : range(bytes.size())) {
        EXPECT_THAT(bytes.at(i), Eq(kAsciiUnknownCodePoint));
    }
}

typedef Test MacRomanEncodingTest;

const char kMacRomanSupplement[] =
        "ÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóòôöõúùûü†°¢£§•¶ß®©™´¨≠ÆØ∞±≤≥¥µ∂∑∏π∫ªºΩæø"
        "¿¡¬√ƒ≈∆«»… ÀÃÕŒœ–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˙˚¸˝˛ˇ";

TEST_F(MacRomanEncodingTest, Decode) {
    Bytes bytes;
    for (int i : range(0x100)) {
        bytes.append(1, i);
    }

    String string(macroman::decode(bytes));
    ASSERT_THAT(string.size(), Eq(bytes.size()));
    for (int i : range(0x80)) {
        EXPECT_THAT(string.at(i), Eq<Rune>(i));
    }
    const String supplement(utf8::decode(kMacRomanSupplement));
    EXPECT_THAT(string.slice(0x80), Eq<StringSlice>(supplement));
}

TEST_F(MacRomanEncodingTest, EncodeValid) {
    String string;
    for (int i : range(0x80)) {
        string.append(1, i);
    }
    string.append(utf8::decode(kMacRomanSupplement));

    Bytes bytes(macroman::encode(string));
    ASSERT_THAT(bytes.size(), Eq(string.size()));
    for (int i : range(bytes.size())) {
        EXPECT_THAT(bytes.at(i), Eq<uint8_t>(i));
    }
}

TEST_F(MacRomanEncodingTest, EncodeInvalid) {
    const String supplement(utf8::decode(kMacRomanSupplement));
    String       string;
    for (int i : range(0x8, 0x100)) {
        if (supplement.find(i * 0x10) == String::npos) {
            string.append(1, i * 0x10);
        } else {
            string.append(1, kUnknownCodePoint);
        }
    }

    Bytes bytes(macroman::encode(string));
    ASSERT_THAT(bytes.size(), Eq(string.size()));
    for (int i : range(bytes.size())) {
        EXPECT_THAT(bytes.at(i), Eq<uint8_t>(kAsciiUnknownCodePoint));
    }
}

typedef Test Utf8EncodingTest;

TEST_F(Utf8EncodingTest, EncodeAscii) {
    String string;
    for (int i : range(0x80)) {
        string.append(1, i);
    }

    Bytes bytes(utf8::encode(string));
    ASSERT_THAT(bytes.size(), Eq(string.size()));
    for (int i : range(bytes.size())) {
        EXPECT_THAT(bytes.at(i), Eq<uint8_t>(i));
    }
}

TEST_F(Utf8EncodingTest, EncodeLatin1) {
    static const char kLatin1Supplement[] =
            "\302\200\302\201\302\202\302\203\302\204\302\205\302\206\302\207"
            "\302\210\302\211\302\212\302\213\302\214\302\215\302\216\302\217"
            "\302\220\302\221\302\222\302\223\302\224\302\225\302\226\302\227"
            "\302\230\302\231\302\232\302\233\302\234\302\235\302\236\302\237"
            "\302\240¡¢£¤¥¦§¨©ª«¬\302\255®¯°±²³´µ¶·¸¹º»¼½¾¿"
            "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";

    String string;
    for (int i : range(0x80, 0x100)) {
        string.append(1, i);
    }

    Bytes bytes(utf8::encode(string));
    ASSERT_THAT(bytes, Eq<BytesSlice>(kLatin1Supplement));
}

TEST_F(Utf8EncodingTest, EncodeCJK) {
    static const char kHiragana[] =
            "ぁあぃいぅうぇえぉおかがきぎくぐけげこご"
            "さざしじすずせぜそぞただちぢっつづてでとど"
            "なにぬねのはばぱひびぴふぶぷへべぺほぼぽ"
            "まみむめもゃやゅゆょよらりるれろゎわゐゑをん";

    String string;
    for (int i : range(0x3041, 0x3094)) {
        string.append(1, i);
    }

    Bytes bytes(utf8::encode(string));
    ASSERT_THAT(bytes, Eq<BytesSlice>(kHiragana));
}

TEST_F(Utf8EncodingTest, EncodeHigherPlane) {
    static const char kHigherPlane[] =
            // Mahjong tiles.
            "\360\237\200\200\360\237\200\201\360\237\200\202\360\237\200\203"
            "\360\237\200\204\360\237\200\205\360\237\200\206\360\237\200\207"
            "\360\237\200\210\360\237\200\211\360\237\200\212\360\237\200\213"
            "\360\237\200\214\360\237\200\215\360\237\200\216\360\237\200\217"
            "\360\237\200\220\360\237\200\221\360\237\200\222\360\237\200\223"
            "\360\237\200\224\360\237\200\225\360\237\200\226\360\237\200\227"
            "\360\237\200\230\360\237\200\231\360\237\200\232\360\237\200\233"
            "\360\237\200\234\360\237\200\235\360\237\200\236\360\237\200\237"
            "\360\237\200\240\360\237\200\241\360\237\200\242\360\237\200\243"
            "\360\237\200\244\360\237\200\245\360\237\200\246\360\237\200\247"
            "\360\237\200\250\360\237\200\251"
            // The last valid unicode character.
            "\364\217\277\277";

    String string;
    for (int i : range(0x1f000, 0x1f02a)) {
        string.append(1, i);
    }
    string.append(1, 0x10ffff);

    Bytes bytes(utf8::encode(string));
    ASSERT_THAT(bytes, Eq<BytesSlice>(kHigherPlane));
}

TEST_F(Utf8EncodingTest, DecodeInvalid) {
    // An array of byte sequences to test utf8::decode() against invalid data.  All ASCII bytes in
    // these strings are implicitly valid, and will be checked to have passed through correctly;
    // non-ASCII bytes all participate in invalid byte sequences, and should be replaced with the
    // replacement character.
    const char kInvalid[][5] = {
            // Invalid bytes.  These can never appear in a valid UTF-8 string, as they would, if
            // anything, start a multi-byte code point that decoded to a value greater than
            // 0x10ffff.
            "\365", "\366", "\367", "\370", "\371", "\372", "\373", "\374", "\375", "\376", "\377",

            // Overlong encoding.  These strings all define interpretable multi-byte code points,
            // but
            // they would decode to a value which could be encoded by fewer bytes.
            "\300\200",          // => 0x00
            "\300\277",          // => 0x3f
            "\301\277",          // => 0x7f
            "\340\200\200",      // => 0x0000
            "\340\237\277",      // => 0x07ff
            "\360\200\200\200",  // => 0x000000
            "\360\217\277\277",  // => 0x00ffff

            // Premature end of string.  Each of the first characters in this string declares a
            // multi-byte character longer than the string being decoded.  Decoding should end
            // early.
            "\300", "\337", "\340", "\340\200", "\357", "\357\277", "\360", "\360\200",
            "\360\200\200", "\364", "\364\277", "\364\277\277",

            // Premature end of code point.  Each of the first characters in this string declares a
            // multi-byte character equal to the length of the string being decoded.  Decoding
            // should
            // continue with the valid bytes.
            "\300A", "\337A", "\340AB", "\340\200A", "\357AB", "\357\277A", "\360ABC",
            "\360\200AB", "\360\200\200A", "\364ABC", "\364\277AB", "\364\277\277A",

            // Continuation bytes not preceded by a header byte.
            "\200", "\201", "\220", "\270", "\277",

            // Surrogate code points.  Strings may not contain code points in the range
            // 0xd800..0xdfff.
            // If a byte sequence would decode to one of these, it is instead invalid.
            "\355\240\200",  // => 0xd800
            "\355\257\277",  // => 0xdbff
            "\355\260\200",  // => 0xdc00
            "\355\277\277",  // => 0xdfff

            // Past plane 16.  Code points starting with \364 could decode to a value greater than
            // 0x10ffff, but should not.  Also, even though we already tested that \365-\367 can't
            // appear in a UTF-8 string above, try anyway.
            "\364\220\200\200",  // => 0x110000
            "\364\277\277\277",  // => 0x13ffff
            "\365\200\200\200",  // => 0x140000
            "\366\200\200\200",  // => 0x180000
            "\367\200\200\200",  // => 0x1c0000
            "\367\277\277\277",  // => 0x1fffff
    };

    for (BytesSlice bytes : kInvalid) {
        String string(utf8::decode(bytes));
        ASSERT_THAT(string.size(), Eq(bytes.size()));
        for (size_t i : range(string.size())) {
            if (bytes.at(i) < 0x80) {
                EXPECT_THAT(string.at(i), Eq(bytes.at(i)));
            } else {
                EXPECT_THAT(string.at(i), Eq(kUnknownCodePoint));
            }
        }
    }
}

}  // namespace
}  // namespace sfz
