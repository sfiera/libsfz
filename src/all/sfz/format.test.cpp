// Copyright (c) 2009-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/format.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pn/string>

using testing::Eq;
using testing::Ne;
using testing::Test;

namespace sfz {
namespace {

struct TestData {
    pn::string      value;
    pn::string_view expected;
};

class FormatterTest : public Test {
  protected:
    template <int size>
    void Run(const TestData (&data)[size]) {
        for (const TestData& test : data) {
            EXPECT_THAT(test.value, Eq(test.expected));
        }
    }
};

TEST_F(FormatterTest, IntFormatterBase) {
    const TestData data[] = {
            // Binary.
            {bin(0), "0"},
            {bin(5), "101"},
            {bin(0xffffffffffffffffull),
             "11111111111111111111111111111111"
             "11111111111111111111111111111111"},
            // Octal.
            {oct(00), "0"},
            {oct(012345670), "12345670"},
            {oct(0xffffffffffffffffull), "1777777777777777777777"},
            // Hexadecimal.
            {hex(0x0), "0"},
            {hex(0x123456789abcdef0ull), "123456789abcdef0"},
            {hex(0xffffffffffffffffull), "ffffffffffffffff"},
    };
    Run(data);
}

TEST_F(FormatterTest, IntFormatterWidth) {
    const TestData data[] = {
            {bin(21, 3), "10101"}, {oct(21, 3), "025"}, {dec(21, 3), "021"}, {hex(21, 3), "015"},
    };
    Run(data);
}

}  // namespace
}  // namespace sfz
