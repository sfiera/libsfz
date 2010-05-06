// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/PrintItem.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"

using testing::Eq;
using testing::Ne;
using testing::Test;

namespace other {

enum Grapheme {
    AESC,
    ETH,
    THORN,
    WYNN,
    YOGH,
};

struct Mapping {
    Grapheme grapheme;
    sfz::Rune code_point;
};

void print_to(sfz::PrintTarget out, Grapheme grapheme) {
    switch (grapheme) {
      case AESC: out.append("AESC"); break;
      case ETH: out.append("ETH"); break;
      case THORN: out.append("THORN"); break;
      case WYNN: out.append("WYNN"); break;
      case YOGH: out.append("YOGH"); break;
      default: out.append("UNKNOWN"); break;
    }
}

void print_to(sfz::PrintTarget out, const Mapping& mapping) {
    print_to(out, mapping.grapheme);
    out.append(" U+");
    sfz::PrintItem(mapping.code_point, 16, 4).print_to(out);
}

};

namespace sfz {
namespace {

template <typename _TestType, typename _StorageType = _TestType>
struct TestData {
    typedef _StorageType StorageType;
    typedef _TestType TestType;
    StorageType value;
    StringPiece expected;
};

template <typename _TestType, typename _StorageType = _TestType>
struct BaseTestData {
    typedef _TestType TestType;
    typedef _StorageType StorageType;
    StorageType value;
    int base;
    StringPiece expected;
};

bool k32Bit = (sizeof(void*) == 4);
bool k64Bit = (sizeof(void*) == 8);

class PrintItemTest : public Test {
  protected:
    PrintItemTest() {
        EXPECT_TRUE(k32Bit || k64Bit) << "Unknown pointer size";
    }

    template <typename T, int i>
    void Run(const StringPiece& prefix, const T (&data)[i]) {
        foreach (it, range(data, data + i)) {
            typename T::TestType value(it->value);
            String s(prefix);
            PrintItem(value).print_to(&s);
            EXPECT_THAT(s, Eq(it->expected));
        }
    }

    template <typename T, int i>
    void RunBase(const StringPiece& prefix, const T (&data)[i]) {
        foreach (it, range(data, data + i)) {
            typename T::TestType value(it->value);
            String s(prefix);
            PrintItem(value, it->base).print_to(&s);
            EXPECT_THAT(s, Eq(it->expected));
        }
    }
};

TEST_F(PrintItemTest, EmptyItem) {
    String s("empty: ");
    PrintItem().print_to(&s);
    EXPECT_THAT(s, Eq<StringPiece>("empty: "));
}

TEST_F(PrintItemTest, CStringItem) {
    const TestData<const char*> data[] = {
        { "",                    "const char*: " },
        { "s",                   "const char*: s" },
        { "long and\nmultiline", "const char*: long and\nmultiline" },
    };
    Run("const char*: ", data);
}

TEST_F(PrintItemTest, BoolItem) {
    const TestData<bool> data[] = {
        { false, "bool: false" },
        { true,  "bool: true" },
    };
    Run("bool: ", data);
}

TEST_F(PrintItemTest, CharItem) {
    const TestData<char> data[] = {
        { '\001', "char: \001" },
        { '\n',   "char: \n" },
        { ' ',    "char:  " },
        { 'A',    "char: A" },
    };
    Run("char: ", data);
}

TEST_F(PrintItemTest, Int8Item) {
    const TestData<int8_t> data[] = {
        { -128, "int8_t: -128" },
        { -109, "int8_t: -109" },
        { -10,  "int8_t: -10" },
        { 0,    "int8_t: 0" },
        { 10,   "int8_t: 10" },
        { 109,  "int8_t: 109" },
        { 127,  "int8_t: 127" },
    };
    Run("int8_t: ", data);
}

TEST_F(PrintItemTest, Uint8Item) {
    const TestData<uint8_t> data[] = {
        { 0,   "uint8_t: 0" },
        { 10,  "uint8_t: 10" },
        { 109, "uint8_t: 109" },
        { 127, "uint8_t: 127" },
        { 255, "uint8_t: 255" },
    };
    Run("uint8_t: ", data);
}

TEST_F(PrintItemTest, Int16Item) {
    const TestData<int16_t> data[] = {
        { -32768, "int16_t: -32768" },
        { 0,      "int16_t: 0" },
        { 32767,  "int16_t: 32767" },
    };
    Run("int16_t: ", data);
}

TEST_F(PrintItemTest, Uint16Item) {
    const TestData<uint16_t> data[] = {
        { 0,     "uint16_t: 0" },
        { 65535, "uint16_t: 65535" },
    };
    Run("uint16_t: ", data);
}

TEST_F(PrintItemTest, Int32Item) {
    const TestData<int32_t> data[] = {
        { 0x80000000ul, "int32_t: -2147483648" },
        { 0l,           "int32_t: 0" },
        { 0x7fffffffl,  "int32_t: 2147483647" },
    };
    Run("int32_t: ", data);
}

TEST_F(PrintItemTest, Uint32Item) {
    const TestData<uint32_t> data[] = {
        { 0ul,          "uint32_t: 0" },
        { 0xfffffffful, "uint32_t: 4294967295" },
    };
    Run("uint32_t: ", data);
}

TEST_F(PrintItemTest, Int64Item) {
    const TestData<int64_t> data[] = {
        { 0x8000000000000000ull, "int64_t: -9223372036854775808" },
        { 0ll,                   "int64_t: 0" },
        { 0x7fffffffffffffffll,  "int64_t: 9223372036854775807" },
    };
    Run("int64_t: ", data);
}

TEST_F(PrintItemTest, Uint64Item) {
    const TestData<uint64_t> data[] = {
        { 0ull,                  "uint64_t: 0" },
        { 0xffffffffffffffffull, "uint64_t: 18446744073709551615" },
    };
    Run("uint64_t: ", data);
}

TEST_F(PrintItemTest, BaseBinaryItem) {
    const BaseTestData<uint64_t> data[] = {
        // Binary.
        { 0ull, 2,                   "base: 0" },
        { 5ull, 2,                   "base: 101" },
        { 0xffffffffffffffffull, 2,  "base: 11111111111111111111111111111111"
                                           "11111111111111111111111111111111" },
        // Ternary.
        { 0ull, 3,                   "base: 0" },
        { 15ull, 3,                  "base: 120" },
        { 0xffffffffffffffffull, 3,  "base: 11112220022122120101211020120210210211220" },
        // Octal.
        { 00ull, 8,                  "base: 0" },
        { 012345670ull, 8,           "base: 12345670" },
        { 0xffffffffffffffffull, 8,  "base: 1777777777777777777777" },
        // Hexadecimal.
        { 0x0ull, 16,                "base: 0" },
        { 0x123456789abcdef0ull, 16, "base: 123456789abcdef0" },
        { 0xffffffffffffffffull, 16, "base: ffffffffffffffff" },
        // Hexatridecimal.
        { 0ull, 36,                  "base: 0" },
        { 0xffffffffffffffffull, 36, "base: 3w5e11264sgsf" },
    };
    RunBase("base: ", data);
}

TEST_F(PrintItemTest, FloatItem) {
    const TestData<float> data[] = {
        { 1.0,      "float: 1.000000" },
        { 1.234567, "float: 1.234567" },
    };
    Run("float: ", data);
}

TEST_F(PrintItemTest, DoubleItem) {
    const TestData<double> data[] = {
        { 1.0,      "double: 1.000000" },
        { 1.234567, "double: 1.234567" },
    };
    Run("double: ", data);
}

TEST_F(PrintItemTest, PointerItem) {
    void* const other = reinterpret_cast<void*>(-1);
    const TestData<void*> data[] = {
        { NULL,  (k32Bit ? "void*: 00000000" : "void*: 0000000000000000") },
        { other, (k32Bit ? "void*: ffffffff" : "void*: ffffffffffffffff") },
    };
    Run("void*: ", data);
}

TEST_F(PrintItemTest, ExternalPointerItem) {
    other::Mapping* const other = reinterpret_cast<other::Mapping*>(-1);
    const TestData<other::Mapping*> data[] = {
        { NULL,  (k32Bit ? "other::Mapping*: 00000000" : "other::Mapping*: 0000000000000000") },
        { other, (k32Bit ? "other::Mapping*: ffffffff" : "other::Mapping*: ffffffffffffffff") },
    };
    Run("other::Mapping*: ", data);
}

TEST_F(PrintItemTest, StringItem) {
    const TestData<String, StringPiece> data[] = {
        { "",                    "string: " },
        { "s",                   "string: s" },
        { "long and\nmultiline", "string: long and\nmultiline" },
    };
    Run("string: ", data);
}

TEST_F(PrintItemTest, StringPieceItem) {
    const TestData<StringPiece> data[] = {
        { "",                    "string: " },
        { "s",                   "string: s" },
        { "long and\nmultiline", "string: long and\nmultiline" },
    };
    Run("string: ", data);
}

TEST_F(PrintItemTest, ExternalEnumTest) {
    const other::Grapheme invalid = static_cast<other::Grapheme>(-1);
    const TestData<other::Grapheme> data[] = {
        { other::AESC,  "other::Grapheme: AESC" },
        { other::ETH,   "other::Grapheme: ETH" },
        { other::THORN, "other::Grapheme: THORN" },
        { other::WYNN,  "other::Grapheme: WYNN" },
        { other::YOGH,  "other::Grapheme: YOGH" },
        { invalid,      "other::Grapheme: UNKNOWN" },
    };
    Run("other::Grapheme: ", data);
}

TEST_F(PrintItemTest, ExternalStructTest) {
    const TestData<other::Mapping> data[] = {
        { { other::AESC,  0x00e6, }, "other::Mapping: AESC U+00e6" },
        { { other::ETH,   0x00f0, }, "other::Mapping: ETH U+00f0" },
        { { other::THORN, 0x00fe, }, "other::Mapping: THORN U+00fe" },
        { { other::WYNN,  0x01bf, }, "other::Mapping: WYNN U+01bf" },
        { { other::YOGH,  0x021d, }, "other::Mapping: YOGH U+021d" },
    };
    Run("other::Mapping: ", data);
}

}  // namespace
}  // namespace sfz
