// Copyright (c) 2009-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/string-map.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::TypedEq;

namespace sfz {
namespace {

using StringMapTest = testing::Test;

TEST_F(StringMapTest, Basic) {
    StringMap<int> m;
    m["a"] = 1;
    EXPECT_THAT(m.begin()->first, TypedEq<pn::string_view>("a"));
    EXPECT_THAT(m.begin()->second, TypedEq<int>(1));
}

}  // namespace
}  // namespace sfz
