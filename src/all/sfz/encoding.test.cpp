// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/encoding.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pn/data>
#include <pn/string>
#include <sfz/range.hpp>

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
    pn::data data;
    for (uint8_t i : range(0x80)) {
        data += pn::data_view{&i, 1};
    }

    pn::string string = ascii::decode(data);
    ASSERT_THAT(string.size(), Eq(data.size()));
    auto it = string.begin();
    for (uint32_t i : range(string.size())) {
        EXPECT_THAT(*(it++), Eq(pn::rune{i}));
    }
}

TEST_F(AsciiEncodingTest, EncodeValid) {
    pn::string string;
    for (uint32_t i : range(0x80)) {
        string += pn::rune{i};
    }

    pn::data data = ascii::encode(string);
    ASSERT_THAT(data.size(), Eq(string.size()));
    for (uint8_t i : range(data.size())) {
        EXPECT_THAT(data[i], Eq(i));
    }
}

TEST_F(AsciiEncodingTest, DecodeInvalid) {
    pn::data data;
    for (uint8_t i : range(0x80, 0x100)) {
        data += pn::data_view{&i, 1};
    }

    pn::string string = ascii::decode(data);
    ASSERT_THAT(string.size(), Eq(3 * data.size()));
    auto it = string.begin();
    for (int i : range(data.size())) {
        EXPECT_THAT(*(it++), Eq(pn::rune{kUnknownCodePoint}));
    }
}

TEST_F(AsciiEncodingTest, EncodeInvalid) {
    pn::string string;
    for (uint32_t i : range(0x8, 0x100)) {
        string += pn::rune{i * 0x10};
    }

    pn::data data = ascii::encode(string);
    ASSERT_THAT(data.size(), Eq(0x100 - 0x8));
    for (int i : range(data.size())) {
        EXPECT_THAT(data[i], Eq(kAsciiUnknownCodePoint.value()));
    }
}

typedef Test Latin1EncodingTest;

TEST_F(Latin1EncodingTest, Decode) {
    pn::data data;
    for (uint8_t i : range(0x100)) {
        data += pn::data_view{&i, 1};
    }

    pn::string string(latin1::decode(data));
    ASSERT_THAT(string.size(), Eq(0x180));
    auto it = string.begin();
    for (uint32_t i : range(data.size())) {
        EXPECT_THAT(*(it++), Eq(pn::rune{i}));
    }
}

TEST_F(Latin1EncodingTest, EncodeValid) {
    pn::string string;
    for (uint32_t i : range(0x100)) {
        string += pn::rune{i};
    }

    pn::data data(latin1::encode(string));
    ASSERT_THAT(data.size(), Eq(0x100));
    for (uint8_t i : range(data.size())) {
        EXPECT_THAT(data[i], Eq(i));
    }
}

TEST_F(Latin1EncodingTest, EncodeInvalid) {
    pn::string string;
    for (uint32_t i : range(0x1, 0x100)) {
        // Some of these generated code points will be in the surrogate code point range, meaning
        // they cannot be appended to the string.  Append kUnknownCodePoint instead, which is
        // valid, but unencodable, so it will end up being encoded as kAsciiUnknownCodePoint.
        if (is_valid_code_point(i * 0x100)) {
            string += pn::rune{i * 0x100};
        } else {
            string += pn::rune{kUnknownCodePoint};
        }
    }

    pn::data data(latin1::encode(string));
    ASSERT_THAT(data.size(), Eq(0x100 - 0x1));
    for (int i : range(data.size())) {
        EXPECT_THAT(data[i], Eq(kAsciiUnknownCodePoint.value()));
    }
}

typedef Test MacRomanEncodingTest;

const pn::string_view kMacRomanSupplement =
        "ÄÅÇÉÑÖÜáàâäãåçéèêëíìîïñóòôöõúùûü†°¢£§•¶ß®©™´¨≠ÆØ∞±≤≥¥µ∂∑∏π∫ªºΩæø"
        "¿¡¬√ƒ≈∆«»… ÀÃÕŒœ–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ‡·‚„‰ÂÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˙˚¸˝˛ˇ";

TEST_F(MacRomanEncodingTest, Decode) {
    pn::data data;
    for (uint8_t i : range(0x100)) {
        data += pn::data_view{&i, 1};
    }

    pn::string string(macroman::decode(data));
    ASSERT_THAT(string.size(), Eq(0x80 + kMacRomanSupplement.size()));
    auto it = string.begin();
    for (uint8_t i : range(0x80)) {
        EXPECT_THAT(*(it++), Eq(pn::rune{i}));
    }
    EXPECT_THAT(string.substr(0x80), Eq(kMacRomanSupplement));
}

TEST_F(MacRomanEncodingTest, EncodeValid) {
    pn::string string;
    for (uint32_t i : range(0x80)) {
        string += pn::rune{i};
    }
    string += kMacRomanSupplement;

    pn::data data(macroman::encode(string));
    ASSERT_THAT(data.size(), Eq(0x100));
    for (uint8_t i : range(data.size())) {
        EXPECT_THAT(data[i], Eq(i));
    }
}

TEST_F(MacRomanEncodingTest, EncodeInvalid) {
    pn::string string;
    for (uint32_t i : range(0x8, 0x100)) {
        if (kMacRomanSupplement.find(pn::rune{i * 0x10}) == kMacRomanSupplement.npos) {
            string += pn::rune{i * 0x10};
        } else {
            string += pn::rune{kUnknownCodePoint};
        }
    }

    pn::data data(macroman::encode(string));
    ASSERT_THAT(data.size(), Eq(0x100 - 0x8));
    for (int i : range(data.size())) {
        EXPECT_THAT(data[i], Eq(kAsciiUnknownCodePoint.value()));
    }
}

typedef Test Utf8EncodingTest;

TEST_F(Utf8EncodingTest, EncodeAscii) {
    pn::string string;
    for (int i : range(0x80)) {
        string += pn::rune(i);
    }

    pn::data bytes{reinterpret_cast<const uint8_t*>(string.data()), string.size()};
    ASSERT_THAT(bytes.size(), Eq(string.size()));
    for (int i : range(bytes.size())) {
        EXPECT_THAT(bytes[i], Eq<uint8_t>(i));
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

    pn::string string;
    for (int i : range(0x80, 0x100)) {
        string += pn::rune(i);
    }

    pn::data bytes{reinterpret_cast<const uint8_t*>(string.data()), string.size()};
    ASSERT_THAT(
            bytes, Eq(pn::data_view{reinterpret_cast<const uint8_t*>(kLatin1Supplement), 256}));
}

}  // namespace
}  // namespace sfz
