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

    String socks("socks", ascii_encoding());
    EXPECT_THAT(string.find(StringPiece()), Eq<size_t>(0));
    EXPECT_THAT(string.find(socks), Eq<size_t>(String::kNone));
    EXPECT_THAT(string.rfind(StringPiece()), Eq<size_t>(0));
    EXPECT_THAT(string.rfind(socks), Eq<size_t>(String::kNone));

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

    const String o("o", ascii_encoding());
    const String orl("orl", ascii_encoding());
    EXPECT_THAT(string.find(StringPiece()), Eq<size_t>(0));
    EXPECT_THAT(string.find(o), Eq<size_t>(4));
    EXPECT_THAT(string.find(orl), Eq<size_t>(8));
    EXPECT_THAT(string.rfind(StringPiece()), Eq<size_t>(13));
    EXPECT_THAT(string.rfind(o), Eq<size_t>(8));
    EXPECT_THAT(string.rfind(orl), Eq<size_t>(8));

    EXPECT_THAT(string, Eq(ByRef(string)));
    EXPECT_THAT(string, Eq<StringPiece>(string));
    EXPECT_THAT(string, Ne(StringPiece()));

    const String hello("Hello", ascii_encoding());
    const String world("world!", ascii_encoding());
    EXPECT_THAT(string.substr(0), Eq<StringPiece>(string));
    EXPECT_THAT(string.substr(0, 5), Eq<StringPiece>(hello));
    EXPECT_THAT(string.substr(7), Eq<StringPiece>(world));
    EXPECT_THAT(string.substr(13), Eq(StringPiece()));
    EXPECT_THAT(string.substr(13, 0), Eq(StringPiece()));
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
        const StringPiece piece(expected);
        const String string(piece);
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
        const String chkchkchk("!!!", ascii_encoding());
        const String string(3, '!');
        EXPECT_THAT(string, Eq<StringPiece>(chkchkchk));
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
        const StringPiece piece(expected);
        String string("Hello, ", ascii_encoding());
        string.assign(piece);
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
        const String chkchkchk("!!!", ascii_encoding());
        String string("Hello, ", ascii_encoding());
        string.assign(3, '!');
        EXPECT_THAT(string, Eq<StringPiece>(chkchkchk));
    }
}

// Test all five overloads of String::append().
TEST_F(StringTest, AllAppendOverloads) {
    const String expected("Hello, world!", ascii_encoding());
    const String append("world!", ascii_encoding());
    {
        String string("Hello, ", ascii_encoding());
        string.append(append);
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        const StringPiece piece(append);
        String string("Hello, ", ascii_encoding());
        string.append(piece);
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        String string("Hello, ", ascii_encoding());
        string.append("world!", ascii_encoding());
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        BytesPiece bytes(reinterpret_cast<const uint8_t*>("world!"), 6);
        String string("Hello, ", ascii_encoding());
        string.append(bytes, ascii_encoding());
        EXPECT_THAT(string, Eq<StringPiece>(expected));
    }
    {
        String string("Hello, world", ascii_encoding());
        string.append(1, '!');
        EXPECT_THAT(string, Eq<StringPiece>(expected));
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

    String goodbye("Goodbye", ascii_encoding());
    string.replace(0, 5, goodbye);
    const String goodbye_world("Goodbye, world!", ascii_encoding());
    EXPECT_THAT(string, Eq<StringPiece>(goodbye_world));

    String cruel("cruel ", ascii_encoding());
    string.replace(9, 0, cruel);
    const String goodbye_cruel_world("Goodbye, cruel world!", ascii_encoding());
    EXPECT_THAT(string, Eq<StringPiece>(goodbye_cruel_world));

    string.replace(20, 1, StringPiece());
    const String clipped("Goodbye, cruel world", ascii_encoding());
    EXPECT_THAT(string, Eq<StringPiece>(clipped));

    String im_leaving_you_today("I'm leaving you today", ascii_encoding());
    string.replace(0, 20, im_leaving_you_today);
    EXPECT_THAT(string, Eq<StringPiece>(im_leaving_you_today));
}

}  // namespace
}  // namespace sfz
