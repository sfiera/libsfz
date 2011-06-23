// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/format.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/bytes.hpp>
#include <sfz/exception.hpp>
#include <sfz/foreach.hpp>

using testing::Eq;
using testing::Ne;
using testing::Test;

namespace sfz {
namespace {

struct TestData {
    PrintItem value;
    StringSlice expected;
};

class FormatterTest : public Test {
  protected:
    template <int size>
    void Run(const StringSlice& prefix, const TestData (&data)[size]) {
        SFZ_FOREACH(const TestData& test, data, {
            String s(prefix);
            test.value.print_to(s);
            EXPECT_THAT(s, Eq(test.expected));
        });
    }
};

TEST_F(FormatterTest, IntFormatterBase) {
    const TestData data[] = {
        // Binary.
        { bin(0),                     "base: 0" },
        { bin(5),                     "base: 101" },
        { bin(0xffffffffffffffffull), "base: 11111111111111111111111111111111"
                                               "11111111111111111111111111111111" },
        // Octal.
        { oct(00),                    "base: 0" },
        { oct(012345670),             "base: 12345670" },
        { oct(0xffffffffffffffffull), "base: 1777777777777777777777" },
        // Hexadecimal.
        { hex(0x0),                   "base: 0" },
        { hex(0x123456789abcdef0ull), "base: 123456789abcdef0" },
        { hex(0xffffffffffffffffull), "base: ffffffffffffffff" },
    };
    Run("base: ", data);
}

TEST_F(FormatterTest, IntFormatterWidth) {
    const TestData data[] = {
        { bin(21, 3), "base: 10101" },
        { oct(21, 3), "base: 025" },
        { dec(21, 3), "base: 021" },
        { hex(21, 3), "base: 015" },
    };
    Run("base: ", data);
}

// Test the 'escape()' formatter.  The three things we want to check are: first, that escaping is
// performed for control and quote characters; second, that when possible, a symbolic name, such as
// '\n', is used; and third, that escapes are always unambiguous, where the original string is
// always reconstructible from the escaped string.
TEST_F(FormatterTest, EscapeFormatter) {
    String nul0;
    nul0.append(1, '\0');
    nul0.append(1, '0');
    const TestData data[] = {
        { escape(""),            "escape: " },
        { escape("normal"),      "escape: normal" },
        { escape("\'\"\\"),      "escape: \\'\\\"\\\\" },
        { escape("multi\nline"), "escape: multi\\nline" },
        { escape("\1\2\3\4\5"),  "escape: \\001\\002\\003\\004\\005" },
        { escape(nul0),          "escape: \\0000" },
    };
    Run("escape: ", data);
}

// The same as the previous test, except in quotes.
TEST_F(FormatterTest, QuoteFormatter) {
    String nul0;
    nul0.append(1, '\0');
    nul0.append(1, '0');
    const TestData data[] = {
        { quote(""),            "quote: \"\"" },
        { quote("normal"),      "quote: \"normal\"" },
        { quote("\'\"\\"),      "quote: \"\\'\\\"\\\\\"" },
        { quote("multi\nline"), "quote: \"multi\\nline\"" },
        { quote("\1\2\3\4\5"),  "quote: \"\\001\\002\\003\\004\\005\"" },
        { quote(nul0),          "quote: \"\\0000\"" },
    };
    Run("quote: ", data);
}

}  // namespace
}  // namespace sfz
