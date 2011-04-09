// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/read.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "sfz/bytes.hpp"
#include "sfz/exception.hpp"
#include "sfz/foreach.hpp"
#include "sfz/format.hpp"

using testing::Eq;
using testing::Test;

namespace other {

enum Grapheme {
    AESC = 0,
    ETH = 1,
    THORN = 2,
    WYNN = 3,
    YOGH = 4,
};

struct Mapping {
    Grapheme grapheme;
    sfz::Rune code_point;
};

void read_from(sfz::ReadSource in, Grapheme* grapheme) {
    uint8_t byte = sfz::read<uint8_t>(&in);
    if (byte <= YOGH) {
        *grapheme = static_cast<Grapheme>(byte);
    } else {
        throw sfz::Exception(sfz::format("invalid Grapheme {0}", byte));
    }
}

void read_from(sfz::ReadSource in, Mapping* mapping) {
    sfz::read(in, &mapping->grapheme);
    sfz::read(in, &mapping->code_point);
}

};

namespace sfz {
namespace {

template <int size>
BytesSlice char_bytes(const char (&data)[size]) {
    return BytesSlice(reinterpret_cast<const uint8_t*>(data), size - 1);
}

typedef Test ReadItemTest;

TEST_F(ReadItemTest, ReadBool) {
    BytesSlice bytes = char_bytes("\001\000\027\001\000\027");
    EXPECT_THAT(read<bool>(&bytes), Eq(true));
    EXPECT_THAT(read<bool>(&bytes), Eq(false));
    EXPECT_THAT(read<bool>(&bytes), Eq(true));
    bool values[3];
    read(&bytes, values, 3);
    EXPECT_THAT(values[0], Eq(true));
    EXPECT_THAT(values[1], Eq(false));
    EXPECT_THAT(values[2], Eq(true));
}

TEST_F(ReadItemTest, ReadChar) {
    BytesSlice bytes = char_bytes("\0000cw\0000t");
    EXPECT_THAT(read<char>(&bytes), Eq('\0'));
    EXPECT_THAT(read<char>(&bytes), Eq('0'));
    EXPECT_THAT(read<char>(&bytes), Eq('c'));
    char values[4];
    read(&bytes, values, 4);
    EXPECT_THAT(values[0], Eq('w'));
    EXPECT_THAT(values[1], Eq('\0'));
    EXPECT_THAT(values[2], Eq('0'));
    EXPECT_THAT(values[3], Eq('t'));
}

TEST_F(ReadItemTest, ReadInt8) {
    BytesSlice bytes = char_bytes("\000\377\001\177\200\000\377\001\177\200");
    EXPECT_THAT(read<int8_t>(&bytes), Eq(0));
    EXPECT_THAT(read<int8_t>(&bytes), Eq(-1));
    EXPECT_THAT(read<int8_t>(&bytes), Eq(1));
    EXPECT_THAT(read<int8_t>(&bytes), Eq(127));
    EXPECT_THAT(read<int8_t>(&bytes), Eq(-128));
    int8_t values[5];
    read(&bytes, values, 5);
    EXPECT_THAT(values[0], Eq(0));
    EXPECT_THAT(values[1], Eq(-1));
    EXPECT_THAT(values[2], Eq(1));
    EXPECT_THAT(values[3], Eq(127));
    EXPECT_THAT(values[4], Eq(-128));
}

TEST_F(ReadItemTest, ReadUint8) {
    BytesSlice bytes = char_bytes("\000\001\177\377\000\001\177\377");
    EXPECT_THAT(read<uint8_t>(&bytes), Eq(0));
    EXPECT_THAT(read<uint8_t>(&bytes), Eq(1));
    EXPECT_THAT(read<uint8_t>(&bytes), Eq(127));
    EXPECT_THAT(read<uint8_t>(&bytes), Eq(255));
    uint8_t values[4];
    read(&bytes, values, 4);
    EXPECT_THAT(values[0], Eq(0));
    EXPECT_THAT(values[1], Eq(1));
    EXPECT_THAT(values[2], Eq(127));
    EXPECT_THAT(values[3], Eq(255));
}

TEST_F(ReadItemTest, ReadLargerInts) {
    BytesSlice bytes = char_bytes(
                    "\000\001" "\000\000\000\002" "\000\000\000\000\000\000\000\003"
                    "\000\144" "\047\020"
                    "\000\000\047\020" "\073\232\312\000"
                    "\000\000\000\000\073\232\312\000" "\015\340\266\263\247\144\000\000");
    EXPECT_THAT(read<int16_t>(&bytes), Eq(1));
    EXPECT_THAT(read<int32_t>(&bytes), Eq(2));
    EXPECT_THAT(read<int64_t>(&bytes), Eq(3));
    uint16_t u16[2];
    uint32_t u32[2];
    uint64_t u64[2];
    read(&bytes, u16, 2);
    read(&bytes, u32, 2);
    read(&bytes, u64, 2);
    EXPECT_THAT(u16[0], Eq(100u));
    EXPECT_THAT(u16[1], Eq(10000u));
    EXPECT_THAT(u32[0], Eq(10000u));
    EXPECT_THAT(u32[1], Eq(1000000000u));
    EXPECT_THAT(u64[0], Eq(1000000000u));
    EXPECT_THAT(u64[1], Eq(1000000000000000000ull));
}

TEST_F(ReadItemTest, ReadExternalEnum) {
    BytesSlice bytes = char_bytes("\000\003\001\003");
    EXPECT_THAT(read<other::Grapheme>(&bytes), Eq(other::AESC));
    EXPECT_THAT(read<other::Grapheme>(&bytes), Eq(other::WYNN));
    other::Grapheme values[2];
    read(&bytes, values, 2);
    EXPECT_THAT(values[0], Eq(other::ETH));
    EXPECT_THAT(values[1], Eq(other::WYNN));
}

TEST_F(ReadItemTest, ReadExternalStruct) {
    BytesSlice bytes = char_bytes(
            "\000\000\000\000\346"
            "\001\000\000\000\360"
            "\002\000\000\000\376"
            "\003\000\000\001\277"
            "\004\000\000\002\035");
    EXPECT_THAT(read<other::Mapping>(&bytes).grapheme, Eq(other::AESC));
    EXPECT_THAT(read<other::Mapping>(&bytes).code_point, Eq(0x00f0u));
    other::Mapping values[3];
    read(&bytes, values, 3);
    EXPECT_THAT(values[0].grapheme, Eq(other::THORN));
    EXPECT_THAT(values[0].code_point, Eq(0x00feu));
    EXPECT_THAT(values[1].grapheme, Eq(other::WYNN));
    EXPECT_THAT(values[1].code_point, Eq(0x01bfu));
    EXPECT_THAT(values[2].grapheme, Eq(other::YOGH));
    EXPECT_THAT(values[2].code_point, Eq(0x021du));
}

}  // namespace
}  // namespace sfz
