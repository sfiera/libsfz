// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/print.hpp"

#include <limits>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Formatter.hpp"
#include "sfz/foreach.hpp"

using std::numeric_limits;
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
      case AESC: out.push("AESC"); break;
      case ETH: out.push("ETH"); break;
      case THORN: out.push("THORN"); break;
      case WYNN: out.push("WYNN"); break;
      case YOGH: out.push("YOGH"); break;
      default: out.push("UNKNOWN"); break;
    }
}

void print_to(sfz::PrintTarget out, const Mapping& mapping) {
    print_to(out, mapping.grapheme);
    out.push(" U+");
    sfz::PrintItem(sfz::hex(mapping.code_point, 4)).print_to(out);
}

};

namespace sfz {
namespace {

template <typename _TestType, typename _StorageType = _TestType>
struct TestData {
    typedef _StorageType StorageType;
    typedef _TestType TestType;
    StorageType value;
    StringSlice expected;
};

template <typename _TestType, typename _StorageType = _TestType>
struct BaseTestData {
    typedef _TestType TestType;
    typedef _StorageType StorageType;
    StorageType value;
    int base;
    StringSlice expected;
};

bool k32Bit = (sizeof(void*) == 4);
bool k64Bit = (sizeof(void*) == 8);

class PrintItemTest : public Test {
  protected:
    PrintItemTest() {
        EXPECT_TRUE(k32Bit || k64Bit) << "Unknown pointer size";
    }

    template <typename T, int size>
    void Run(const StringSlice& prefix, const T (&data)[size]) {
        foreach (const T& test, data) {
            typename T::TestType value(test.value);
            String s(prefix);
            print(&s, value);
            EXPECT_THAT(s, Eq(test.expected));
        }
    }

};

TEST_F(PrintItemTest, EmptyItem) {
    String s("empty: ");
    PrintItem().print_to(&s);
    EXPECT_THAT(s, Eq<StringSlice>("empty: "));
}

TEST_F(PrintItemTest, CStringItem) {
    const TestData<const char*> data[] = {
        { "",                    "const char*: " },
        { "s",                   "const char*: s" },
        { "long and\nmultiline", "const char*: long and\nmultiline" },
    };
    Run("const char*: ", data);
}

TEST_F(PrintItemTest, CStringLiteralItem) {
    {
        String s("const char[1]: ");
        print(&s, "");
        EXPECT_THAT(s, Eq<StringSlice>("const char[1]: "));
    }
    {
        String s("const char[14]: ");
        print(&s, "quoted string");
        EXPECT_THAT(s, Eq<StringSlice>("const char[14]: quoted string"));
    }
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

TEST_F(PrintItemTest, FloatItem) {
    const TestData<float> data[] = {
        { 1.0,      "float: 1.000000" },
        { 1.234567, "float: 1.234567" },
    };
    Run("float: ", data);
}

TEST_F(PrintItemTest, DoubleItem) {
    const TestData<double> data[] = {
        { 0.0,                                  "double: 0.000000" },
        { -0.0,                                 "double: -0.000000" },
        { 1.0,                                  "double: 1.000000" },
        { -1.0,                                 "double: -1.000000" },
        { 1.234567,                             "double: 1.234567" },
        { -1.234567,                            "double: -1.234567" },
        { numeric_limits<double>::infinity(),   "double: inf" },
        { -numeric_limits<double>::infinity(),  "double: -inf" },
        { numeric_limits<double>::quiet_NaN(),  "double: nan" },
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
    const TestData<String, StringSlice> data[] = {
        { "",                    "string: " },
        { "s",                   "string: s" },
        { "long and\nmultiline", "string: long and\nmultiline" },
    };
    Run("string: ", data);
}

TEST_F(PrintItemTest, StringSliceItem) {
    const TestData<StringSlice> data[] = {
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
