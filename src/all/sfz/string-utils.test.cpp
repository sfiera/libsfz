// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/string-utils.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pn/string>
#include <sfz/encoding.hpp>

using testing::Eq;
using testing::NanSensitiveDoubleEq;
using testing::NanSensitiveFloatEq;
using testing::Test;

namespace sfz {
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
