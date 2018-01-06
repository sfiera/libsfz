// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/string-utils.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pn/string>
#include <sfz/encoding.hpp>
#include <sfz/exception.hpp>
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

template <typename T>
struct TestData {
    StringToIntResult::Failure failure;
    const char*                string;
    ValueOrMessage<T>          expected;
};

StringToIntResult::Failure NONE = StringToIntResult::NONE;
StringToIntResult::Failure INVL = StringToIntResult::INVALID_LITERAL;
StringToIntResult::Failure OVER = StringToIntResult::INTEGER_OVERFLOW;

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

class StringUtilitiesTest : public Test {
  protected:
    template <typename T, int size>
    void RunInt(const TestData<T> (&inputs)[size]) {
        for (const TestData<T>& input : inputs) {
            T                 value;
            StringToIntResult result = string_to_int<T>(input.string, value);
            EXPECT_THAT(result, Eq(input.failure == NONE)) << "input: " << input.string;
            EXPECT_THAT(result.failure, Eq(input.failure)) << "input: " << input.string;
            if (input.failure == NONE) {
                EXPECT_THAT(value, Eq(input.expected.value)) << "input: " << input.string;
                EXPECT_THAT(result, PrintsTo("")) << "input: " << input.string;
            } else {
                EXPECT_THAT(result, PrintsTo(input.expected.message)) << "input: " << input.string;
            }
        }
    }

    template <typename T, int size>
    void RunFloat(const TestData<T> (&inputs)[size]) {
        for (const TestData<T>& input : inputs) {
            T actual;
            if (input.failure == StringToIntResult::NONE) {
                EXPECT_THAT(string_to_float(input.string, actual), Eq(true))
                        << "input: " << input.string;
                EXPECT_THAT(actual, NanSensitiveFloatEq(input.expected.value));
            } else {
                EXPECT_THAT(string_to_float(input.string, actual), Eq(false))
                        << "input: " << input.string << "; output: " << actual;
            }
        }
    }

    template <typename T, int size>
    void RunDouble(const TestData<T> (&inputs)[size]) {
        for (const TestData<T>& input : inputs) {
            T actual;
            if (input.failure == StringToIntResult::NONE) {
                EXPECT_THAT(string_to_float(input.string, actual), Eq(true))
                        << "input: " << input.string;
                EXPECT_THAT(actual, NanSensitiveDoubleEq(input.expected.value));
            } else {
                EXPECT_THAT(string_to_float(input.string, actual), Eq(false))
                        << "input: " << input.string << "; output: " << actual;
            }
        }
    }
};

TEST_F(StringUtilitiesTest, Int8) {
    TestData<int8_t> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "10", 10},
            {NONE, "127", 127},
            {OVER, "128", "literal too large for int8_t with base 10"},
            {OVER, "130", "literal too large for int8_t with base 10"},
            {OVER, "1000", "literal too large for int8_t with base 10"},

            {NONE, "-1", -1},
            {NONE, "-10", -10},
            {NONE, "-128", -128},
            {OVER, "-129", "literal too large for int8_t with base 10"},
            {OVER, "-130", "literal too large for int8_t with base 10"},
            {OVER, "-1000", "literal too large for int8_t with base 10"},

            {NONE, "0000000000000000000000000", 0},
            {NONE, "0000000000000000000000001", 1},
            {NONE, "0000000000000000000000127", 127},
            {OVER, "0000000000000000000000128", "literal too large for int8_t with base 10"},

            {INVL, "", "invalid literal for int8_t with base 10"},
            {INVL, "-", "invalid literal for int8_t with base 10"},
            {INVL, "a", "invalid literal for int8_t with base 10"},
            {INVL, "-a", "invalid literal for int8_t with base 10"},
            {INVL, "-1000a", "invalid literal for int8_t with base 10"},
    };
    RunInt(inputs);
}

TEST_F(StringUtilitiesTest, Int16) {
    TestData<int16_t> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "10", 10},
            {NONE, "32767", 32767},
            {OVER, "32768", "literal too large for int16_t with base 10"},
            {OVER, "32770", "literal too large for int16_t with base 10"},
            {OVER, "100000", "literal too large for int16_t with base 10"},

            {NONE, "-1", -1},
            {NONE, "-10", -10},
            {NONE, "-32768", -32768},
            {OVER, "-32769", "literal too large for int16_t with base 10"},
            {OVER, "-32770", "literal too large for int16_t with base 10"},
            {OVER, "-100000", "literal too large for int16_t with base 10"},

            {NONE, "0000000000000000000000000", 0},
            {NONE, "0000000000000000000000001", 1},
            {NONE, "0000000000000000000032767", 32767},
            {OVER, "0000000000000000000032768", "literal too large for int16_t with base 10"},

            {INVL, "", "invalid literal for int16_t with base 10"},
            {INVL, "-", "invalid literal for int16_t with base 10"},
            {INVL, "a", "invalid literal for int16_t with base 10"},
            {INVL, "-a", "invalid literal for int16_t with base 10"},
            {INVL, "-100000a", "invalid literal for int16_t with base 10"},
    };
    RunInt(inputs);
}

TEST_F(StringUtilitiesTest, Int32) {
    TestData<int32_t> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "10", 10},
            {NONE, "2147483647", 2147483647},
            {OVER, "2147483648", "literal too large for int32_t with base 10"},
            {OVER, "2147483650", "literal too large for int32_t with base 10"},
            {OVER, "10000000000", "literal too large for int32_t with base 10"},

            {NONE, "-1", -1},
            {NONE, "-10", -10},
            {NONE, "-2147483648", -2147483648ll},
            {OVER, "-2147483649", "literal too large for int32_t with base 10"},
            {OVER, "-2147483650", "literal too large for int32_t with base 10"},
            {OVER, "-10000000000", "literal too large for int32_t with base 10"},

            {NONE, "0000000000000000000000000", 0},
            {NONE, "0000000000000000000000001", 1},
            {NONE, "0000000000000002147483647", 2147483647ull},
            {OVER, "0000000000000002147483648", "literal too large for int32_t with base 10"},

            {INVL, "", "invalid literal for int32_t with base 10"},
            {INVL, "-", "invalid literal for int32_t with base 10"},
            {INVL, "a", "invalid literal for int32_t with base 10"},
            {INVL, "-a", "invalid literal for int32_t with base 10"},
            {INVL, "-10000000000a", "invalid literal for int32_t with base 10"},
    };
    RunInt(inputs);
}

TEST_F(StringUtilitiesTest, Int64) {
    TestData<int64_t> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "10", 10},
            {NONE, "9223372036854775807", 9223372036854775807ull},
            {OVER, "9223372036854775808", "literal too large for int64_t with base 10"},
            {OVER, "9223372036854775810", "literal too large for int64_t with base 10"},
            {OVER, "10000000000000000000", "literal too large for int64_t with base 10"},

            {NONE, "-1", -1},
            {NONE, "-10", -10},
            {NONE, "-9223372036854775808", -9223372036854775808ull},
            {OVER, "-9223372036854775809", "literal too large for int64_t with base 10"},
            {OVER, "-9223372036854775810", "literal too large for int64_t with base 10"},
            {OVER, "-10000000000000000000", "literal too large for int64_t with base 10"},

            {NONE, "0000000000000000000000000", 0},
            {NONE, "0000000000000000000000001", 1},
            {NONE, "0000009223372036854775807", 9223372036854775807ull},
            {OVER, "0000009223372036854775808", "literal too large for int64_t with base 10"},

            {INVL, "", "invalid literal for int64_t with base 10"},
            {INVL, "-", "invalid literal for int64_t with base 10"},
            {INVL, "a", "invalid literal for int64_t with base 10"},
            {INVL, "-a", "invalid literal for int64_t with base 10"},
            {INVL, "-10000000000000000000a", "invalid literal for int64_t with base 10"},
    };
    RunInt(inputs);
}

TEST_F(StringUtilitiesTest, UnsignedInt8) {
    TestData<uint8_t> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "10", 10},
            {NONE, "127", 127},
            {NONE, "128", 128},
            {NONE, "255", 255},
            {OVER, "256", "literal too large for uint8_t with base 10"},
            {OVER, "260", "literal too large for uint8_t with base 10"},
            {OVER, "1000", "literal too large for uint8_t with base 10"},

            {INVL, "-", "invalid literal for uint8_t with base 10"},
            {INVL, "-1", "invalid literal for uint8_t with base 10"},
            {INVL, "-10", "invalid literal for uint8_t with base 10"},
            {INVL, "-1000", "invalid literal for uint8_t with base 10"},

            {NONE, "0000000000000000000000000", 0},
            {NONE, "0000000000000000000000001", 1},
            {NONE, "0000000000000000000000255", 255},
            {OVER, "0000000000000000000000256", "literal too large for uint8_t with base 10"},

            {INVL, "", "invalid literal for uint8_t with base 10"},
            {INVL, "a", "invalid literal for uint8_t with base 10"},
            {INVL, "1000a", "invalid literal for uint8_t with base 10"},
    };
    RunInt(inputs);
}

TEST_F(StringUtilitiesTest, UnsignedInt16) {
    TestData<uint16_t> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "10", 10},
            {NONE, "32767", 32767},
            {NONE, "32768", 32768},
            {NONE, "65535", 65535},
            {OVER, "65536", "literal too large for uint16_t with base 10"},
            {OVER, "65540", "literal too large for uint16_t with base 10"},
            {OVER, "100000", "literal too large for uint16_t with base 10"},

            {INVL, "-", "invalid literal for uint16_t with base 10"},
            {INVL, "-1", "invalid literal for uint16_t with base 10"},
            {INVL, "-10", "invalid literal for uint16_t with base 10"},
            {INVL, "-100000", "invalid literal for uint16_t with base 10"},

            {NONE, "0000000000000000000000000", 0},
            {NONE, "0000000000000000000000001", 1},
            {NONE, "0000000000000000000065535", 65535},
            {OVER, "0000000000000000000065536", "literal too large for uint16_t with base 10"},

            {INVL, "", "invalid literal for uint16_t with base 10"},
            {INVL, "a", "invalid literal for uint16_t with base 10"},
            {INVL, "100000a", "invalid literal for uint16_t with base 10"},
    };
    RunInt(inputs);
}

TEST_F(StringUtilitiesTest, UnsignedInt32) {
    TestData<uint32_t> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "10", 10},
            {NONE, "2147483647", 2147483647},
            {NONE, "2147483648", 2147483648ull},
            {NONE, "4294967295", 4294967295ull},
            {OVER, "4294967296", "literal too large for uint32_t with base 10"},
            {OVER, "4294967300", "literal too large for uint32_t with base 10"},
            {OVER, "10000000000", "literal too large for uint32_t with base 10"},

            {INVL, "-", "invalid literal for uint32_t with base 10"},
            {INVL, "-1", "invalid literal for uint32_t with base 10"},
            {INVL, "-10", "invalid literal for uint32_t with base 10"},
            {INVL, "-10000000000", "invalid literal for uint32_t with base 10"},

            {NONE, "0000000000000000000000000", 0},
            {NONE, "0000000000000000000000001", 1},
            {NONE, "0000000000000004294967295", 4294967295ull},
            {OVER, "0000000000000004294967296", "literal too large for uint32_t with base 10"},

            {INVL, "", "invalid literal for uint32_t with base 10"},
            {INVL, "a", "invalid literal for uint32_t with base 10"},
            {INVL, "10000000000a", "invalid literal for uint32_t with base 10"},
    };
    RunInt(inputs);
}

TEST_F(StringUtilitiesTest, UnsignedInt64) {
    TestData<uint64_t> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "10", 10},
            {NONE, "9223372036854775807", 9223372036854775807ull},
            {NONE, "9223372036854775808", 9223372036854775808ull},
            {NONE, "18446744073709551615", 18446744073709551615ull},
            {OVER, "18446744073709551616", "literal too large for uint64_t with base 10"},
            {OVER, "18446744073709551620", "literal too large for uint64_t with base 10"},
            {OVER, "100000000000000000000", "literal too large for uint64_t with base 10"},

            {INVL, "-", "invalid literal for uint64_t with base 10"},
            {INVL, "-1", "invalid literal for uint64_t with base 10"},
            {INVL, "-10", "invalid literal for uint64_t with base 10"},
            {INVL, "-100000000000000000000", "invalid literal for uint64_t with base 10"},

            {NONE, "0000000000000000000000000", 0},
            {NONE, "0000000000000000000000001", 1},
            {NONE, "0000018446744073709551615", 18446744073709551615ull},
            {OVER, "0000018446744073709551616", "literal too large for uint64_t with base 10"},

            {INVL, "", "invalid literal for uint64_t with base 10"},
            {INVL, "a", "invalid literal for uint64_t with base 10"},
            {INVL, "100000000000000000000a", "invalid literal for uint64_t with base 10"},
    };
    RunInt(inputs);
}

TEST_F(StringUtilitiesTest, Float) {
    TestData<float> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "-1", -1},
            {NONE, "1.5", 1.5},
            {NONE, "1e10", 1e10},
            {NONE, "infinity", std::numeric_limits<float>::infinity()},
            {NONE, "-infinity", -std::numeric_limits<float>::infinity()},
            {NONE, "nan", std::numeric_limits<float>::quiet_NaN()},
    };
    RunFloat(inputs);
}

TEST_F(StringUtilitiesTest, Double) {
    TestData<double> inputs[] = {
            {NONE, "0", 0},
            {NONE, "1", 1},
            {NONE, "-1", -1},
            {NONE, "1.5", 1.5},
            {NONE, "1e10", 1e10},
            {NONE, "infinity", std::numeric_limits<float>::infinity()},
            {NONE, "-infinity", -std::numeric_limits<float>::infinity()},
            {NONE, "nan", std::numeric_limits<float>::quiet_NaN()},
    };
    RunDouble(inputs);
}

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
