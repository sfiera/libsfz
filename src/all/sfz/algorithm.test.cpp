// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/algorithm.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/bytes.hpp>
#include <sfz/string.hpp>

using testing::Test;
using testing::Eq;

namespace sfz {

typedef testing::Test AlgorithmTest;

TEST_F(AlgorithmTest, Int) {
    const int from = 1;
    int to = 0;
    copy(to, from);
    ASSERT_THAT(to, Eq(from));
}

TEST_F(AlgorithmTest, BytesSlice) {
    const BytesSlice from("from");
    BytesSlice to("to");
    copy(to, from);
    ASSERT_THAT(to, Eq(from));
}

TEST_F(AlgorithmTest, Bytes) {
    const Bytes from("from");
    Bytes to("to");
    copy(to, from);
    ASSERT_THAT(to, Eq<BytesSlice>(from));
}

TEST_F(AlgorithmTest, StringSlice) {
    const StringSlice from("from");
    StringSlice to("to");
    copy(to, from);
    ASSERT_THAT(to, Eq(from));
}

TEST_F(AlgorithmTest, String) {
    const String from("from");
    String to("to");
    copy(to, from);
    ASSERT_THAT(to, Eq<StringSlice>(from));
}

}  // namespace sfz
