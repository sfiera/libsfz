// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/bytes.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "sfz/encoding.hpp"
#include "sfz/exception.hpp"
#include "sfz/string.hpp"

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

typedef Test BytesTest;

// Test all of the 'const' methods of an empty Bytes.
TEST_F(BytesTest, EmptyConst) {
    const Bytes bytes;

    EXPECT_THAT(bytes.size(), Eq<size_t>(0));
    EXPECT_THAT(bytes.empty(), Eq(true));

    EXPECT_THROW(bytes.at(0), Exception);

    EXPECT_THAT(bytes.find('\0'), Eq(Bytes::npos));
    EXPECT_THAT(bytes.rfind('\0'), Eq(Bytes::npos));

    EXPECT_THAT(bytes.find(""), Eq<size_t>(0));
    EXPECT_THAT(bytes.find("socks"), Eq<size_t>(Bytes::npos));
    EXPECT_THAT(bytes.rfind(""), Eq<size_t>(0));
    EXPECT_THAT(bytes.rfind("socks"), Eq<size_t>(Bytes::npos));

    EXPECT_THAT(bytes, Eq(ByRef(bytes)));
    EXPECT_THAT(bytes, Eq<BytesSlice>(bytes));
    EXPECT_THAT(bytes, Eq(BytesSlice()));

    EXPECT_THAT(bytes.slice(0), Eq(BytesSlice()));
    EXPECT_THAT(bytes.slice(0, 0), Eq(BytesSlice()));
    EXPECT_THROW(bytes.slice(1), Exception);
    EXPECT_THROW(bytes.slice(0, 1), Exception);
    EXPECT_THROW(bytes.slice(1, 0), Exception);
}

// Test all of the 'const' methods of a non-empty Bytes.
TEST_F(BytesTest, HelloWorldConst) {
    const Bytes bytes("Hello, world!");

    EXPECT_THAT(bytes.size(), Eq<size_t>(13));
    EXPECT_THAT(bytes.empty(), Eq(false));

    EXPECT_THAT(bytes.at(0), Eq<uint8_t>('H'));
    EXPECT_THAT(bytes.at(4), Eq<uint8_t>('o'));
    EXPECT_THAT(bytes.at(8), Eq<uint8_t>('o'));
    EXPECT_THAT(bytes.at(12), Eq<uint8_t>('!'));
    EXPECT_THROW(bytes.at(13), Exception);

    EXPECT_THAT(bytes.find('H'), Eq<size_t>(0));
    EXPECT_THAT(bytes.find('o'), Eq<size_t>(4));
    EXPECT_THAT(bytes.find('!'), Eq<size_t>(12));
    EXPECT_THAT(bytes.rfind('H'), Eq<size_t>(0));
    EXPECT_THAT(bytes.rfind('o'), Eq<size_t>(8));
    EXPECT_THAT(bytes.rfind('!'), Eq<size_t>(12));

    EXPECT_THAT(bytes.find(""), Eq<size_t>(0));
    EXPECT_THAT(bytes.find("o"), Eq<size_t>(4));
    EXPECT_THAT(bytes.find("orl"), Eq<size_t>(8));
    EXPECT_THAT(bytes.rfind(""), Eq<size_t>(13));
    EXPECT_THAT(bytes.rfind("o"), Eq<size_t>(8));
    EXPECT_THAT(bytes.rfind("orl"), Eq<size_t>(8));

    EXPECT_THAT(bytes, Eq(ByRef(bytes)));
    EXPECT_THAT(bytes, Eq<BytesSlice>(bytes));
    EXPECT_THAT(bytes, Ne(BytesSlice()));

    EXPECT_THAT(bytes.slice(0), Eq<BytesSlice>(bytes));
    EXPECT_THAT(bytes.slice(0, 5), Eq("Hello"));
    EXPECT_THAT(bytes.slice(7), Eq("world!"));
    EXPECT_THAT(bytes.slice(13), Eq(BytesSlice("")));
    EXPECT_THAT(bytes.slice(13, 0), Eq(BytesSlice("")));
    EXPECT_THROW(bytes.slice(14), Exception);
    EXPECT_THROW(bytes.slice(13, 1), Exception);
    EXPECT_THROW(bytes.slice(14, 0), Exception);
}

// Test all five non-default overloads of Bytes's constructor.
TEST_F(BytesTest, AllNonEmptyConstructors) {
    const char* expected = "Hello, world!";
    {
        const Bytes s(expected);
        const Bytes bytes(s);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        const BytesSlice s(expected);
        const Bytes bytes(s);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        const Bytes bytes(expected);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        StringSlice string(expected);
        const Bytes bytes(ascii::encode(string));
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        const Bytes bytes(3, '!');
        EXPECT_THAT(bytes, Eq("!!!"));
    }
}

// Test all five overloads of Bytes::assign().
TEST_F(BytesTest, AllAssignOverloads) {
    const char* expected = "Hello, world!";
    {
        Bytes s(expected);
        Bytes bytes("Hello, ");
        bytes.assign(s);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        BytesSlice s(expected);
        Bytes bytes("Hello, ");
        bytes.assign(s);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        Bytes bytes("Hello, ");
        bytes.assign(expected);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        StringSlice string(expected);
        Bytes bytes("Hello, ");
        bytes.assign(ascii::encode(string));
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        Bytes bytes("Hello, ");
        bytes.assign(3, '!');
        EXPECT_THAT(bytes, Eq("!!!"));
    }
}

// Test all five overloads of Bytes::append().
TEST_F(BytesTest, AllAppendOverloads) {
    const char* expected = "Hello, world!";
    const char* append = "world!";
    {
        Bytes s(append);
        Bytes bytes("Hello, ");
        bytes.append(s);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        BytesSlice s(append);
        Bytes bytes("Hello, ");
        bytes.append(s);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        Bytes bytes("Hello, ");
        bytes.append(append);
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        StringSlice string("world!");
        Bytes bytes("Hello, ");
        bytes.append(ascii::encode(string));
        EXPECT_THAT(bytes, Eq(expected));
    }
    {
        Bytes bytes("Hello, world");
        bytes.append(1, '!');
        EXPECT_THAT(bytes, Eq(expected));
    }
}

// Test Bytes's ability to resize itself by appending exclamation points, one at a time, until it
// is 100 code points long.
TEST_F(BytesTest, ExtensionByOnes) {
    Bytes bytes;
    for (int i = 0; i < 100; ++i) {
        bytes.append(1, '!');
        EXPECT_THAT(bytes.size(), Eq<size_t>(i + 1));
        const Bytes expected(i + 1, '!');
        EXPECT_THAT(bytes, Eq<BytesSlice>(expected));
    }
}

// Test Bytes's ability to resize itself by appending exclamation points, doubling the number
// appended each time, such that it is tested with each size that is a power of two from 2 ** 1 to
// 2 ** 14.
TEST_F(BytesTest, ExtensionByPowersOfTwo) {
    Bytes bytes(1, '!');
    for (int i = 0; i < 13; ++i) {
        bytes.append(1 << i, '!');
        EXPECT_THAT(bytes.size(), Eq<size_t>(2 << i));
        const Bytes expected(2 << i, '!');
        EXPECT_THAT(bytes, Eq<BytesSlice>(expected));
    }
}

// Test Bytes::replace().
TEST_F(BytesTest, Replace) {
    Bytes bytes("Hello, world!");

    bytes.replace(0, 5, "Goodbye");
    EXPECT_THAT(bytes, Eq("Goodbye, world!"));

    bytes.replace(9, 0, "cruel ");
    EXPECT_THAT(bytes, Eq("Goodbye, cruel world!"));

    bytes.replace(20, 1, "");
    EXPECT_THAT(bytes, Eq("Goodbye, cruel world"));

    bytes.replace(0, 20, "I'm leaving you today");
    EXPECT_THAT(bytes, Eq("I'm leaving you today"));
}

template <typename Left, typename Right>
void TestComparison() {
    const Bytes hello_bytes("Hello");
    const Bytes world_bytes("World");
    const Bytes backspace_bytes("\377");

    Left world_left(world_bytes);
    Right hello(hello_bytes);
    Right world_right(world_bytes);
    Right backspace(backspace_bytes);

    EXPECT_THAT(world_left, Ne(ByRef(hello)));
    EXPECT_THAT(world_left, Eq(ByRef(world_right)));
    EXPECT_THAT(world_left, Ne(ByRef(backspace)));

    EXPECT_THAT(world_left, Gt(ByRef(hello)));
    EXPECT_THAT(world_left, Le(ByRef(world_right)));
    EXPECT_THAT(world_left, Le(ByRef(backspace)));

    EXPECT_THAT(world_left, Ge(ByRef(hello)));
    EXPECT_THAT(world_left, Ge(ByRef(world_right)));
    EXPECT_THAT(world_left, Lt(ByRef(backspace)));
}

TEST_F(BytesTest, Comparison) {
    TestComparison<Bytes,      Bytes>();
    TestComparison<Bytes,      BytesSlice>();
    TestComparison<BytesSlice, BytesSlice>();
}

}  // namespace
}  // namespace sfz
