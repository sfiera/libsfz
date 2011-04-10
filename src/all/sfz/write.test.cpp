// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/write.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/bytes.hpp>
#include <sfz/print.hpp>
#include <sfz/foreach.hpp>

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

void write_to(sfz::WriteTarget out, Grapheme grapheme) {
    sfz::write<uint8_t>(out, grapheme);
}

void write_to(sfz::WriteTarget out, const Mapping& mapping) {
    sfz::write(out, mapping.grapheme);
    sfz::write(out, mapping.code_point);
}

};

namespace sfz {
namespace {

template <int size>
BytesSlice char_bytes(const char (&data)[size]) {
    return BytesSlice(reinterpret_cast<const uint8_t*>(data), size - 1);
}

typedef Test WriteItemTest;

TEST_F(WriteItemTest, WriteBool) {
    Bytes bytes;
    write<bool>(&bytes, true);
    write<bool>(&bytes, false);
    write<bool>(&bytes, 23);
    bool values[] = { true, false, 23 };
    write(&bytes, values, 3);
    EXPECT_THAT(bytes, Eq(char_bytes("\001\000\001\001\000\001")));
}

TEST_F(WriteItemTest, WriteChar) {
    Bytes bytes;
    write(&bytes, '\0');
    write(&bytes, '0');
    write(&bytes, 'c');
    write(&bytes, "w\000t", 3);
    EXPECT_THAT(bytes, Eq(char_bytes("\0000cw\000t")));
}

TEST_F(WriteItemTest, WriteInt8) {
    Bytes bytes;
    write<int8_t>(&bytes, 0);
    write<int8_t>(&bytes, -1);
    write<int8_t>(&bytes, 1);
    write<int8_t>(&bytes, 127);
    write<int8_t>(&bytes, -128);
    int8_t values[] = { 0, -1, 1, 127, -128 };
    write(&bytes, values, 5);
    EXPECT_THAT(bytes, Eq(char_bytes("\000\377\001\177\200\000\377\001\177\200")));
}

TEST_F(WriteItemTest, WriteUint8) {
    Bytes bytes;
    write<uint8_t>(&bytes, 0);
    write<uint8_t>(&bytes, 1);
    write<uint8_t>(&bytes, 127);
    write<uint8_t>(&bytes, 255);
    uint8_t values[] = { 0, 1, 127, 255 };
    write(&bytes, values, 4);
    EXPECT_THAT(bytes, Eq(char_bytes("\000\001\177\377\000\001\177\377")));
}

TEST_F(WriteItemTest, WriteLargerInts) {
    Bytes bytes;
    write<int16_t>(&bytes, 1);
    write<int32_t>(&bytes, 2);
    write<int64_t>(&bytes, 3);
    uint16_t u16[] = { 100, 10000 };
    uint32_t u32[] = { 10000, 1000000000 };
    uint64_t u64[] = { 1000000000, 1000000000000000000ull };
    write(&bytes, u16, 2);
    write(&bytes, u32, 2);
    write(&bytes, u64, 2);
    EXPECT_THAT(bytes, Eq(char_bytes(
                    "\000\001" "\000\000\000\002" "\000\000\000\000\000\000\000\003"
                    "\000\144" "\047\020"
                    "\000\000\047\020" "\073\232\312\000"
                    "\000\000\000\000\073\232\312\000" "\015\340\266\263\247\144\000\000")));
}

TEST_F(WriteItemTest, WriteBytes) {
    Bytes bytes;
    write(&bytes, char_bytes("\000\001\002"));
    write(&bytes, bytes);
    EXPECT_THAT(bytes, Eq(char_bytes("\000\001\002\000\001\002")));
}

TEST_F(WriteItemTest, WriteExternalEnum) {
    Bytes bytes;
    write(&bytes, other::AESC);
    write(&bytes, other::WYNN);
    other::Grapheme values[] = { other::ETH, other::WYNN };
    write(&bytes, values, 2);
    EXPECT_THAT(bytes, Eq(char_bytes("\000\003\001\003")));
}

TEST_F(WriteItemTest, WriteExternalStruct) {
    Bytes bytes;
    other::Mapping values[] = {
        { other::AESC,  0x00e6 },
        { other::ETH,   0x00f0 },
        { other::THORN, 0x00fe },
        { other::WYNN,  0x01bf },
        { other::YOGH,  0x021d },
    };
    write(&bytes, values[0]);
    write(&bytes, values + 1, 4);
    EXPECT_THAT(bytes, Eq(char_bytes(
                    "\000\000\000\000\346"
                    "\001\000\000\000\360"
                    "\002\000\000\000\376"
                    "\003\000\000\001\277"
                    "\004\000\000\002\035")));
}

}  // namespace
}  // namespace sfz
