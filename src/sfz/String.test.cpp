// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/String.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"

using testing::ByRef;
using testing::Eq;
using testing::Ne;
using testing::Test;

namespace sfz {
namespace {

typedef Test StringTest;

// Test all of the 'const' methods of an empty String.
TEST_F(StringTest, EmptyConst) {
    const String string;

    EXPECT_THAT(string.size(), Eq<size_t>(0));
    EXPECT_THAT(string.empty(), Eq(true));

    EXPECT_THROW(string.at(0), Exception);

    EXPECT_THAT(string.find('\0'), Eq(String::kNone));
    EXPECT_THAT(string.rfind('\0'), Eq(String::kNone));

    EXPECT_THAT(string.find(StringPiece()), Eq<size_t>(0));
    EXPECT_THAT(string.find("socks"), Eq<size_t>(String::kNone));
    EXPECT_THAT(string.rfind(StringPiece()), Eq<size_t>(0));
    EXPECT_THAT(string.rfind("socks"), Eq<size_t>(String::kNone));

    EXPECT_THAT(string, Eq(ByRef(string)));
    EXPECT_THAT(string, Eq<StringPiece>(string));
    EXPECT_THAT(string, Eq(StringPiece()));

    EXPECT_THAT(string.substr(0), Eq(StringPiece()));
    EXPECT_THAT(string.substr(0, 0), Eq(StringPiece()));
    EXPECT_THROW(string.substr(1), Exception);
    EXPECT_THROW(string.substr(0, 1), Exception);
    EXPECT_THROW(string.substr(1, 0), Exception);
}

// Test all of the 'const' methods of a non-empty String.
TEST_F(StringTest, HelloWorldConst) {
    const String string("Hello, world!", ascii_encoding());

    EXPECT_THAT(string.size(), Eq<size_t>(13));
    EXPECT_THAT(string.empty(), Eq(false));

    EXPECT_THAT(string.at(0), Eq<uint32_t>('H'));
    EXPECT_THAT(string.at(4), Eq<uint32_t>('o'));
    EXPECT_THAT(string.at(8), Eq<uint32_t>('o'));
    EXPECT_THAT(string.at(12), Eq<uint32_t>('!'));
    EXPECT_THROW(string.at(13), Exception);

    EXPECT_THAT(string.find('H'), Eq<size_t>(0));
    EXPECT_THAT(string.find('o'), Eq<size_t>(4));
    EXPECT_THAT(string.find('!'), Eq<size_t>(12));
    EXPECT_THAT(string.rfind('H'), Eq<size_t>(0));
    EXPECT_THAT(string.rfind('o'), Eq<size_t>(8));
    EXPECT_THAT(string.rfind('!'), Eq<size_t>(12));

    EXPECT_THAT(string.find(""), Eq<size_t>(0));
    EXPECT_THAT(string.find("o"), Eq<size_t>(4));
    EXPECT_THAT(string.find("orl"), Eq<size_t>(8));
    EXPECT_THAT(string.rfind(""), Eq<size_t>(13));
    EXPECT_THAT(string.rfind("o"), Eq<size_t>(8));
    EXPECT_THAT(string.rfind("orl"), Eq<size_t>(8));

    EXPECT_THAT(string, Eq(ByRef(string)));
    EXPECT_THAT(string, Eq<StringPiece>(string));
    EXPECT_THAT(string, Ne(StringPiece()));

    EXPECT_THAT(string.substr(0), Eq<StringPiece>(string));
    EXPECT_THAT(string.substr(0, 5), Eq<StringPiece>("Hello"));
    EXPECT_THAT(string.substr(7), Eq<StringPiece>("world!"));
    EXPECT_THAT(string.substr(13), Eq(StringPiece("")));
    EXPECT_THAT(string.substr(13, 0), Eq(StringPiece("")));
    EXPECT_THROW(string.substr(14), Exception);
    EXPECT_THROW(string.substr(13, 1), Exception);
    EXPECT_THROW(string.substr(14, 0), Exception);
}

// Test all five non-default overloads of String's constructor.
TEST_F(StringTest, AllNonEmptyConstructors) {
    const String expected("Hello, world!", ascii_encoding());
    {
        const String string(expected);
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        const String string("Hello, world!");
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        const String string("Hello, world!", ascii_encoding());
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        BytesPiece bytes(reinterpret_cast<const uint8_t*>("Hello, world!"), 13);
        const String string(bytes, ascii_encoding());
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        const String string(3, '!');
        EXPECT_THAT(string, Eq<StringPiece>("!!!"));
    }
}

// Test all five overloads of String::assign().
TEST_F(StringTest, AllAssignOverloads) {
    const String expected("Hello, world!", ascii_encoding());
    {
        String string("Hello, ", ascii_encoding());
        string.assign(expected);
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        String string("Hello, ", ascii_encoding());
        string.assign("Hello, world!");
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        String string("Hello, ", ascii_encoding());
        string.assign("Hello, world!", ascii_encoding());
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        BytesPiece bytes(reinterpret_cast<const uint8_t*>("Hello, world!"), 13);
        String string("Hello, ", ascii_encoding());
        string.assign(bytes, ascii_encoding());
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        String string("Hello, ", ascii_encoding());
        string.assign(3, '!');
        EXPECT_THAT(string, Eq<StringPiece>("!!!"));
    }
}

// Test all five overloads of String::append().
TEST_F(StringTest, AllAppendOverloads) {
    const String expected("Hello, world!", ascii_encoding());
    const String append("world!", ascii_encoding());
    {
        String string("Hello, ", ascii_encoding());
        string.append(append);
        EXPECT_THAT(string, Eq<StringPiece>("Hello, world!"));
    }
    {
        String string("Hello, ", ascii_encoding());
        string.append("world!");
        EXPECT_THAT(string, Eq<StringPiece>("Hello, world!"));
    }
    {
        String string("Hello, ", ascii_encoding());
        string.append("world!", ascii_encoding());
        EXPECT_THAT(string, Eq<StringPiece>("Hello, world!"));
    }
    {
        BytesPiece bytes(reinterpret_cast<const uint8_t*>("world!"), 6);
        String string("Hello, ", ascii_encoding());
        string.append(bytes, ascii_encoding());
        EXPECT_THAT(string, Eq<StringPiece>("Hello, world!"));
    }
    {
        String string("Hello, world", ascii_encoding());
        string.append(1, '!');
        EXPECT_THAT(string, Eq<StringPiece>("Hello, world!"));
    }
}

// Test String's ability to resize itself by appending exclamation points, one at a time, until it
// is 100 code points long.
TEST_F(StringTest, ExtensionByOnes) {
    String string;
    for (int i = 0; i < 100; ++i) {
        string.append(1, '!');
        EXPECT_THAT(string.size(), Eq<size_t>(i + 1));
        const String expected(i + 1, '!');
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
}

// Test String's ability to resize itself by appending exclamation points, doubling the number
// appended each time, such that it is tested with each size that is a power of two from 2 ** 0 to
// 2 ** 14.
TEST_F(StringTest, ExtensionByPowersOfTwo) {
    String string(1, '!');
    for (int i = 0; i < 13; ++i) {
        string.append(1 << i, '!');
        EXPECT_THAT(string.size(), Eq<size_t>(2 << i));
        const String expected(2 << i, '!');
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
}

// Test String::replace().
TEST_F(StringTest, Replace) {
    String string("Hello, world!", ascii_encoding());

    string.replace(0, 5, "Goodbye");
    EXPECT_THAT(string, Eq<StringPiece>("Goodbye, world!"));

    string.replace(9, 0, "cruel ");
    EXPECT_THAT(string, Eq<StringPiece>("Goodbye, cruel world!"));

    string.replace(20, 1, "");
    EXPECT_THAT(string, Eq<StringPiece>("Goodbye, cruel world"));

    string.replace(0, 20, "I'm leaving you today");
    EXPECT_THAT(string, Eq<StringPiece>("I'm leaving you today"));
}

TEST_F(StringTest, ReplaceGreek) {
    const String kalimera_kosme_bang("Καλημέρα, κόσμε!", utf8_encoding());
    String string(kalimera_kosme_bang);

    const String antio("Αντίο", utf8_encoding());
    const String antio_kosme_bang("Αντίο, κόσμε!", utf8_encoding());
    string.replace(0, 8, antio);
    EXPECT_THAT(string, Eq<StringPiece>(antio_kosme_bang));

    const String sklere("σκλερέ ", utf8_encoding());
    const String antio_sklere_kosme_bang("Αντίο, σκλερέ κόσμε!", utf8_encoding());
    string.replace(7, 0, sklere);
    EXPECT_THAT(string, Eq<StringPiece>(antio_sklere_kosme_bang));

    const String antio_sklere_kosme("Αντίο, σκλερέ κόσμε", utf8_encoding());
    string.replace(19, 1, "");
    EXPECT_THAT(string, Eq<StringPiece>(antio_sklere_kosme));

    const String sas_afino_simera("Σας αφήνω σήμερα", utf8_encoding());
    string.replace(0, 19, sas_afino_simera);
    EXPECT_THAT(string, Eq<StringPiece>(sas_afino_simera));
}

TEST_F(StringTest, ReplaceJapanese) {
    const String konnitiha_sekai_bang("こんにちは世界！", utf8_encoding());
    String string(konnitiha_sekai_bang);

    const String sayonara("さよなら", utf8_encoding());
    const String sayonara_sekai_bang("さよなら世界！", utf8_encoding());
    string.replace(0, 5, sayonara);
    EXPECT_THAT(string, Eq<StringPiece>(sayonara_sekai_bang));

    const String hidoi("ひどい", utf8_encoding());
    const String sayonara_hidoi_sekai_bang("さよならひどい世界！", utf8_encoding());
    string.replace(4, 0, hidoi);
    EXPECT_THAT(string, Eq<StringPiece>(sayonara_hidoi_sekai_bang));

    const String sayonara_hidoi_sekai("さよならひどい世界", utf8_encoding());
    string.replace(9, 1, "");
    EXPECT_THAT(string, Eq<StringPiece>(sayonara_hidoi_sekai));

    const String kyouha_anatawo_nokoshiteimasu("今日あなたを残しています", utf8_encoding());
    string.replace(0, 9, kyouha_anatawo_nokoshiteimasu);
    EXPECT_THAT(string, Eq<StringPiece>(kyouha_anatawo_nokoshiteimasu));
}

}  // namespace
}  // namespace sfz
