// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/string-utils.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/foreach.hpp>
#include <sfz/string.hpp>

using testing::Eq;
using testing::Test;

namespace sfz {
namespace {

enum Goodness {
    GOOD = true,
    BAD = false,
};

template <typename T>
struct TestData {
    Goodness good;
    const char* string;
    T expected;
};

class StringUtilitiesTest : public Test {
  protected:
    template <typename T, int size>
    void Run(const TestData<T> (&inputs)[size]) {
        SFZ_FOREACH(const TestData<T>& input, inputs, {
            T actual;
            if (input.good) {
                EXPECT_THAT(string_to_int(input.string, &actual), Eq(true))
                    << "input: " << input.string;
                EXPECT_THAT(actual, Eq(input.expected));
            } else {
                EXPECT_THAT(string_to_int(input.string, &actual), Eq(false))
                    << "input: " << input.string << "; output: " << actual;
            }
        });
    }
};

TEST_F(StringUtilitiesTest, Int8) {
    TestData<int8_t> inputs[] = {
        {GOOD, "0",                         0},
        {GOOD, "1",                         1},
        {GOOD, "10",                        10},
        {GOOD, "127",                       127},
        {BAD,  "128"},
        {BAD,  "130"},
        {BAD,  "1000"},
        {GOOD, "-1",                        -1},
        {GOOD, "-10",                       -10},
        {GOOD, "-128",                      -128},
        {BAD,  "-129"},
        {BAD,  "-130"},
        {BAD,  "-1000"},
        {GOOD, "0000000000000000000000000", 0},
        {GOOD, "0000000000000000000000001", 1},
        {GOOD, "0000000000000000000000127", 127},
        {BAD,  "0000000000000000000000128"},
    };
    Run(inputs);
}

TEST_F(StringUtilitiesTest, Int16) {
    TestData<int16_t> inputs[] = {
        {GOOD, "0",                         0},
        {GOOD, "1",                         1},
        {GOOD, "10",                        10},
        {GOOD, "32767",                     32767},
        {BAD,  "32768"},
        {BAD,  "32770"},
        {BAD,  "100000"},
        {GOOD, "-1",                        -1},
        {GOOD, "-10",                       -10},
        {GOOD, "-32768",                    -32768},
        {BAD,  "-32769"},
        {BAD,  "-32770"},
        {BAD,  "-100000"},
        {GOOD, "0000000000000000000000000", 0},
        {GOOD, "0000000000000000000000001", 1},
        {GOOD, "0000000000000000000032767", 32767},
        {BAD,  "0000000000000000000032768"},
    };
    Run(inputs);
}

TEST_F(StringUtilitiesTest, Int32) {
    TestData<int32_t> inputs[] = {
        {GOOD, "0",                         0},
        {GOOD, "1",                         1},
        {GOOD, "10",                        10},
        {GOOD, "2147483647",                2147483647},
        {BAD,  "2147483648"},
        {BAD,  "2147483650"},
        {BAD,  "10000000000"},
        {GOOD, "-1",                        -1},
        {GOOD, "-10",                       -10},
        {GOOD, "-2147483648",               -2147483648ll},
        {BAD,  "-2147483649"},
        {BAD,  "-2147483650"},
        {BAD,  "-10000000000"},
        {GOOD, "0000000000000000000000000", 0},
        {GOOD, "0000000000000000000000001", 1},
        {GOOD, "0000000000000002147483647", 2147483647ull},
        {BAD,  "0000000000000002147483648"},
    };
    Run(inputs);
}

TEST_F(StringUtilitiesTest, Int64) {
    TestData<int64_t> inputs[] = {
        {GOOD, "0",                         0},
        {GOOD, "1",                         1},
        {GOOD, "10",                        10},
        {GOOD, "9223372036854775807",       9223372036854775807ull},
        {BAD,  "9223372036854775808"},
        {BAD,  "9223372036854775810"},
        {BAD,  "10000000000000000000"},
        {GOOD, "-1",                        -1},
        {GOOD, "-10",                       -10},
        {GOOD, "-9223372036854775808",      -9223372036854775808ull},
        {BAD,  "-9223372036854775809"},
        {BAD,  "-9223372036854775810"},
        {BAD,  "-10000000000000000000"},
        {GOOD, "0000000000000000000000000", 0},
        {GOOD, "0000000000000000000000001", 1},
        {GOOD, "0000009223372036854775807", 9223372036854775807ull},
        {BAD,  "0000009223372036854775808"},
    };
    Run(inputs);
}

TEST_F(StringUtilitiesTest, UnsignedInt8) {
    TestData<uint8_t> inputs[] = {
        {GOOD, "0",                         0},
        {GOOD, "1",                         1},
        {GOOD, "10",                        10},
        {GOOD, "127",                       127},
        {GOOD, "128",                       128},
        {GOOD, "255",                       255},
        {BAD,  "256"},
        {BAD,  "260"},
        {BAD,  "1000"},
        {BAD,  "-1",                        -1},
        {BAD,  "-10",                       -10},
        {BAD,  "-1000"},
        {GOOD, "0000000000000000000000000", 0},
        {GOOD, "0000000000000000000000001", 1},
        {GOOD, "0000000000000000000000255", 255},
        {BAD,  "0000000000000000000000256"},
    };
    Run(inputs);
}

TEST_F(StringUtilitiesTest, UnsignedInt16) {
    TestData<uint16_t> inputs[] = {
        {GOOD, "0",                         0},
        {GOOD, "1",                         1},
        {GOOD, "10",                        10},
        {GOOD, "32767",                     32767},
        {GOOD, "32768",                     32768},
        {GOOD, "65535",                     65535},
        {BAD,  "65536"},
        {BAD,  "65540"},
        {BAD,  "100000"},
        {BAD,  "-1",                        -1},
        {BAD,  "-10",                       -10},
        {BAD,  "-100000"},
        {GOOD, "0000000000000000000000000", 0},
        {GOOD, "0000000000000000000000001", 1},
        {GOOD, "0000000000000000000065535", 65535},
        {BAD,  "0000000000000000000065536"},
    };
    Run(inputs);
}

TEST_F(StringUtilitiesTest, UnsignedInt32) {
    TestData<uint32_t> inputs[] = {
        {GOOD, "0",                         0},
        {GOOD, "1",                         1},
        {GOOD, "10",                        10},
        {GOOD, "2147483647",                2147483647},
        {GOOD, "2147483648",                2147483648ull},
        {GOOD, "4294967295",                4294967295ull},
        {BAD,  "4294967296"},
        {BAD,  "4294967300"},
        {BAD,  "10000000000"},
        {BAD,  "-1",                        -1},
        {BAD,  "-10",                       -10},
        {BAD,  "-10000000000"},
        {GOOD, "0000000000000000000000000", 0},
        {GOOD, "0000000000000000000000001", 1},
        {GOOD, "0000000000000004294967295", 4294967295ull},
        {BAD,  "0000000000000004294967296"},
    };
    Run(inputs);
}

TEST_F(StringUtilitiesTest, UnsignedInt64) {
    TestData<uint64_t> inputs[] = {
        {GOOD, "0",                         0},
        {GOOD, "1",                         1},
        {GOOD, "10",                        10},
        {GOOD, "9223372036854775807",       9223372036854775807ull},
        {GOOD, "9223372036854775808",       9223372036854775808ull},
        {GOOD, "18446744073709551615",      18446744073709551615ull},
        {BAD,  "18446744073709551616"},
        {BAD,  "18446744073709551620"},
        {BAD,  "100000000000000000000"},
        {BAD,  "-1",                        -1},
        {BAD,  "-10",                       -10},
        {BAD,  "-100000000000000000000"},
        {GOOD, "0000000000000000000000000", 0},
        {GOOD, "0000000000000000000000001", 1},
        {GOOD, "0000018446744073709551615", 18446744073709551615ull},
        {BAD,  "0000018446744073709551616"},
    };
    Run(inputs);
}

}  // namespace
}  // namespace sfz
