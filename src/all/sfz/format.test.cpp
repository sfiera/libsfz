// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/format.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/bytes.hpp>

using testing::Eq;
using testing::Ne;
using testing::Test;

namespace sfz {
namespace {

struct TestData {
    PrintItem   value;
    StringSlice expected;
};

class FormatterTest : public Test {
  protected:
    template <int size>
    void Run(const StringSlice& prefix, const TestData (&data)[size]) {
        for (const TestData& test : data) {
            String s(prefix);
            test.value.print_to(s);
            EXPECT_THAT(s, Eq(test.expected));
        }
    }
};

TEST_F(FormatterTest, IntFormatterBase) {
    const TestData data[] = {
            // Binary.
            {bin(0), "base: 0"},
            {bin(5), "base: 101"},
            {bin(0xffffffffffffffffull),
             "base: 11111111111111111111111111111111"
             "11111111111111111111111111111111"},
            // Octal.
            {oct(00), "base: 0"},
            {oct(012345670), "base: 12345670"},
            {oct(0xffffffffffffffffull), "base: 1777777777777777777777"},
            // Hexadecimal.
            {hex(0x0), "base: 0"},
            {hex(0x123456789abcdef0ull), "base: 123456789abcdef0"},
            {hex(0xffffffffffffffffull), "base: ffffffffffffffff"},
    };
    Run("base: ", data);
}

TEST_F(FormatterTest, IntFormatterWidth) {
    const TestData data[] = {
            {bin(21, 3), "base: 10101"},
            {oct(21, 3), "base: 025"},
            {dec(21, 3), "base: 021"},
            {hex(21, 3), "base: 015"},
    };
    Run("base: ", data);
}

}  // namespace
}  // namespace sfz
