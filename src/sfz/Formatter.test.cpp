// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Formatter.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"

using testing::Eq;
using testing::Ne;
using testing::Test;

namespace sfz {
namespace {

struct TestData {
    PrintItem value;
    StringPiece expected;
};

class FormatterTest : public Test {
  protected:
    template <int i>
    void Run(const StringPiece& prefix, const TestData (&data)[i]) {
        foreach (it, range(data, data + i)) {
            String s(prefix);
            it->value.print_to(&s);
            EXPECT_THAT(s, Eq(it->expected));
        }
    }
};

// Formatters are just syntactic sugar for the 3-arg integral constructors of PrintItem.  They're
// exercised more fully in that test, so we don't bother with an exhaustive test suite here.
TEST_F(FormatterTest, BaseFormatters) {
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
