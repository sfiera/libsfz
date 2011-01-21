// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Io.hpp"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::Test;

namespace sfz {
namespace {

typedef Test IoTest;

TEST_F(IoTest, Compiles) {
    // No verification of results, but it compiles.
    print(io::out, "Test");
}

}  // namespace
}  // namespace sfz
