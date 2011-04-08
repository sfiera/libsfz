// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/string.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "sfz/Exception.hpp"
#include "sfz/bytes.hpp"
#include "sfz/encoding.hpp"

using testing::ByRef;
using testing::Eq;
using testing::Ge;
using testing::Gt;
using testing::Le;
using testing::Lt;
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

    EXPECT_THAT(string.find('\0'), Eq(String::npos));
    EXPECT_THAT(string.rfind('\0'), Eq(String::npos));

    EXPECT_THAT(string.find(StringSlice()), Eq<size_t>(0));
    EXPECT_THAT(string.find("socks"), Eq<size_t>(String::npos));
    EXPECT_THAT(string.rfind(StringSlice()), Eq<size_t>(0));
    EXPECT_THAT(string.rfind("socks"), Eq<size_t>(String::npos));

    EXPECT_THAT(string, Eq(ByRef(string)));
    EXPECT_THAT(string, Eq<StringSlice>(string));
    EXPECT_THAT(string, Eq(StringSlice()));

    EXPECT_THAT(string.slice(0), Eq(StringSlice()));
    EXPECT_THAT(string.slice(0, 0), Eq(StringSlice()));
    EXPECT_THROW(string.slice(1), Exception);
    EXPECT_THROW(string.slice(0, 1), Exception);
    EXPECT_THROW(string.slice(1, 0), Exception);
}

// Test all of the 'const' methods of a non-empty String.
TEST_F(StringTest, HelloWorldConst) {
    const String string("Hello, world!");

    EXPECT_THAT(string.size(), Eq<size_t>(13));
    EXPECT_THAT(string.empty(), Eq(false));

    EXPECT_THAT(string.at(0), Eq<Rune>('H'));
    EXPECT_THAT(string.at(4), Eq<Rune>('o'));
    EXPECT_THAT(string.at(8), Eq<Rune>('o'));
    EXPECT_THAT(string.at(12), Eq<Rune>('!'));
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
    EXPECT_THAT(string, Eq<StringSlice>(string));
    EXPECT_THAT(string, Ne(StringSlice()));

    EXPECT_THAT(string.slice(0), Eq<StringSlice>(string));
    EXPECT_THAT(string.slice(0, 5), Eq("Hello"));
    EXPECT_THAT(string.slice(7), Eq("world!"));
    EXPECT_THAT(string.slice(13), Eq(StringSlice("")));
    EXPECT_THAT(string.slice(13, 0), Eq(StringSlice("")));
    EXPECT_THROW(string.slice(14), Exception);
    EXPECT_THROW(string.slice(13, 1), Exception);
    EXPECT_THROW(string.slice(14, 0), Exception);
}

// Test all five non-default overloads of String's constructor.
TEST_F(StringTest, AllNonEmptyConstructors) {
    const char* expected = "Hello, world!";
    {
        const String s(expected);
        const String string(s);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        const StringSlice s(expected);
        const String string(s);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        const String string(expected);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        BytesSlice bytes(reinterpret_cast<const uint8_t*>(expected), strlen(expected));
        const String string(ascii::decode(bytes));
        EXPECT_THAT(string, Eq(expected));
    }
    {
        const String string(3, '!');
        EXPECT_THAT(string, Eq("!!!"));
    }
}

// Test all five overloads of String::assign().
TEST_F(StringTest, AllAssignOverloads) {
    const char* expected = "Hello, world!";
    {
        String s(expected);
        String string("Hello, ");
        string.assign(s);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        StringSlice s(expected);
        String string("Hello, ");
        string.assign(s);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        String string("Hello, ");
        string.assign(expected);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        BytesSlice bytes(reinterpret_cast<const uint8_t*>(expected), strlen(expected));
        String string("Hello, ");
        string.assign(ascii::decode(bytes));
        EXPECT_THAT(string, Eq(expected));
    }
    {
        String string("Hello, ");
        string.assign(3, '!');
        EXPECT_THAT(string, Eq("!!!"));
    }
}

// Test all five overloads of String::append().
TEST_F(StringTest, AllAppendOverloads) {
    const char* expected = "Hello, world!";
    const char* append = "world!";
    {
        String s(append);
        String string("Hello, ");
        string.append(s);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        StringSlice s(append);
        String string("Hello, ");
        string.append(s);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        String string("Hello, ");
        string.append(append);
        EXPECT_THAT(string, Eq(expected));
    }
    {
        BytesSlice bytes(reinterpret_cast<const uint8_t*>("world!"), 6);
        String string("Hello, ");
        string.append(ascii::decode(bytes));
        EXPECT_THAT(string, Eq(expected));
    }
    {
        String string("Hello, world");
        string.append(1, '!');
        EXPECT_THAT(string, Eq(expected));
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
        EXPECT_THAT(string, Eq<StringSlice>(expected));
    }
}

// Test String's ability to resize itself by appending exclamation points, doubling the number
// appended each time, such that it is tested with each size that is a power of two from 2 ** 1 to
// 2 ** 14.
TEST_F(StringTest, ExtensionByPowersOfTwo) {
    String string(1, '!');
    for (int i = 0; i < 13; ++i) {
        string.append(1 << i, '!');
        EXPECT_THAT(string.size(), Eq<size_t>(2 << i));
        const String expected(2 << i, '!');
        EXPECT_THAT(string, Eq<StringSlice>(expected));
    }
}

// Test String::replace().
TEST_F(StringTest, Replace) {
    String string("Hello, world!");

    string.replace(0, 5, "Goodbye");
    EXPECT_THAT(string, Eq("Goodbye, world!"));

    string.replace(9, 0, "cruel ");
    EXPECT_THAT(string, Eq("Goodbye, cruel world!"));

    string.replace(20, 1, "");
    EXPECT_THAT(string, Eq("Goodbye, cruel world"));

    string.replace(0, 20, "I'm leaving you today");
    EXPECT_THAT(string, Eq("I'm leaving you today"));
}

TEST_F(StringTest, ReplaceGreek) {
    const String kalimera_kosme_bang(utf8::decode("Καλημέρα, κόσμε!"));
    String string(kalimera_kosme_bang);

    const String antio(utf8::decode("Αντίο"));
    const String antio_kosme_bang(utf8::decode("Αντίο, κόσμε!"));
    string.replace(0, 8, antio);
    EXPECT_THAT(string, Eq<StringSlice>(antio_kosme_bang));

    const String sklere(utf8::decode("σκλερέ "));
    const String antio_sklere_kosme_bang(utf8::decode("Αντίο, σκλερέ κόσμε!"));
    string.replace(7, 0, sklere);
    EXPECT_THAT(string, Eq<StringSlice>(antio_sklere_kosme_bang));

    const String antio_sklere_kosme(utf8::decode("Αντίο, σκλερέ κόσμε"));
    string.replace(19, 1, "");
    EXPECT_THAT(string, Eq<StringSlice>(antio_sklere_kosme));

    const String sas_afino_simera(utf8::decode("Σας αφήνω σήμερα"));
    string.replace(0, 19, sas_afino_simera);
    EXPECT_THAT(string, Eq<StringSlice>(sas_afino_simera));
}

TEST_F(StringTest, ReplaceJapanese) {
    const String konnitiha_sekai_bang(utf8::decode("こんにちは世界！"));
    String string(konnitiha_sekai_bang);

    const String sayonara(utf8::decode("さよなら"));
    const String sayonara_sekai_bang(utf8::decode("さよなら世界！"));
    string.replace(0, 5, sayonara);
    EXPECT_THAT(string, Eq<StringSlice>(sayonara_sekai_bang));

    const String hidoi(utf8::decode("ひどい"));
    const String sayonara_hidoi_sekai_bang(utf8::decode("さよならひどい世界！"));
    string.replace(4, 0, hidoi);
    EXPECT_THAT(string, Eq<StringSlice>(sayonara_hidoi_sekai_bang));

    const String sayonara_hidoi_sekai(utf8::decode("さよならひどい世界"));
    string.replace(9, 1, "");
    EXPECT_THAT(string, Eq<StringSlice>(sayonara_hidoi_sekai));

    const String kyouha_anatawo_nokositeimasu(utf8::decode("今日あなたを残しています"));
    string.replace(0, 9, kyouha_anatawo_nokositeimasu);
    EXPECT_THAT(string, Eq<StringSlice>(kyouha_anatawo_nokositeimasu));
}

template <typename Left, typename Right>
void TestComparison() {
    const String hello_string("Hello");
    const String world_string("World");
    const String kalimera_string(utf8::decode("Καλημέρα"));

    Left world_left(world_string);
    Right hello(hello_string);
    Right world_right(world_string);
    Right kalimera(kalimera_string);

    EXPECT_THAT(world_left, Ne(ByRef(hello)));
    EXPECT_THAT(world_left, Eq(ByRef(world_right)));
    EXPECT_THAT(world_left, Ne(ByRef(kalimera)));

    EXPECT_THAT(world_left, Gt(ByRef(hello)));
    EXPECT_THAT(world_left, Le(ByRef(world_right)));
    EXPECT_THAT(world_left, Le(ByRef(kalimera)));

    EXPECT_THAT(world_left, Ge(ByRef(hello)));
    EXPECT_THAT(world_left, Ge(ByRef(world_right)));
    EXPECT_THAT(world_left, Lt(ByRef(kalimera)));
}

TEST_F(StringTest, Comparison) {
    TestComparison<String,      String>();
    TestComparison<String,      StringSlice>();
    TestComparison<StringSlice, StringSlice>();
}

}  // namespace
}  // namespace sfz
