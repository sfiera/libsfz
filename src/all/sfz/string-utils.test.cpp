// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/string-utils.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pn/string>
#include <sfz/encoding.hpp>
#include <sfz/string.hpp>

using testing::Eq;
using testing::NanSensitiveDoubleEq;
using testing::NanSensitiveFloatEq;
using testing::Test;

namespace sfz {

void PrintTo(const String& s, std::ostream* ostr) {
    CString c_str(s);
    *ostr << c_str.data();
}

void PrintTo(StringSlice s, std::ostream* ostr) {
    CString c_str(s);
    *ostr << c_str.data();
}

namespace {

template <typename T>
struct ValueOrMessage {
    template <typename U>
    ValueOrMessage(U u) : value(u), message(NULL) {}
    ValueOrMessage(const char* m) : value(0), message(m) {}

    T           value;
    const char* message;
};

template <>
struct ValueOrMessage<const char*> {
    ValueOrMessage(const char* m) : value(m), message(m) {}

    const char* value;
    const char* message;
};

MATCHER_P(PrintsTo, expected, "") {
    sfz::String actual(arg);
    sfz::String actual_quoted(actual);
    CString     actual_c_str(actual_quoted);
    sfz::String expected_quoted(expected);
    CString     expected_c_str(expected_quoted);
    *result_listener << "actual " << actual_c_str.data() << " vs. expected "
                     << expected_c_str.data();
    return actual == expected;
}

using StringUtilitiesTest = ::testing::Test;

TEST_F(StringUtilitiesTest, Upper) {
    std::pair<pn::string_view, pn::string_view> inputs[] = {
            {"", ""},         {"a", "A"},         {"Na", "NA"},         {"WTF", "WTF"},
            {"w00t", "W00T"}, {"Ελένη", "Ελένη"}, {"林さん", "林さん"},
    };
    for (const auto& input : inputs) {
        pn::string      actual   = upper(input.first);
        pn::string_view expected = input.second;
        EXPECT_THAT(actual, Eq(expected))
                << "input: " << input.first.copy().c_str() << "; actual: " << actual.c_str();
    }
}

TEST_F(StringUtilitiesTest, Lower) {
    std::pair<pn::string_view, pn::string_view> inputs[] = {
            {"", ""},         {"A", "a"},         {"Na", "na"},         {"ill", "ill"},
            {"HNO2", "hno2"}, {"Ελένη", "Ελένη"}, {"林さん", "林さん"},
    };
    for (const auto& input : inputs) {
        pn::string      actual   = lower(input.first);
        pn::string_view expected = input.second;
        EXPECT_THAT(actual, Eq(expected))
                << "input: " << input.first.copy().c_str() << "; actual: " << actual.c_str();
    }
}

}  // namespace
}  // namespace sfz
