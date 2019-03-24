// Copyright (c) 2011-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/optional.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pn/string>
#include <stdexcept>

using testing::Eq;
using testing::Test;

namespace sfz {
namespace {

typedef Test OptionalTest;

template <typename T>
const T& const_(T& t) {
    return t;
}
template <typename T>
const T* const_(T* t) {
    return t;
}

TEST_F(OptionalTest, Int) {
    optional<int> o;
    EXPECT_THAT(o.has_value(), Eq(false));
    EXPECT_THROW(o.value(), std::runtime_error);
    EXPECT_THROW(const_(o).value(), std::runtime_error);
    EXPECT_THAT(o.value_or(1), Eq(1));
    EXPECT_THAT(const_(o).value_or(1), Eq(1));

    o = nullopt;
    EXPECT_THAT(o.has_value(), Eq(false));
    EXPECT_THROW(o.value(), std::runtime_error);
    EXPECT_THROW(const_(o).value(), std::runtime_error);
    EXPECT_THAT(o.value_or(1), Eq(1));
    EXPECT_THAT(const_(o).value_or(1), Eq(1));

    o.reset();
    EXPECT_THAT(o.has_value(), Eq(false));
    EXPECT_THROW(o.value(), std::runtime_error);
    EXPECT_THROW(const_(o).value(), std::runtime_error);
    EXPECT_THAT(o.value_or(1), Eq(1));
    EXPECT_THAT(const_(o).value_or(1), Eq(1));

    o.emplace();
    EXPECT_THAT(o.has_value(), Eq(true));
    EXPECT_THAT(o.value(), Eq(0));
    EXPECT_THAT(const_(o).value(), Eq(0));
    EXPECT_THAT(*o, Eq(0));
    EXPECT_THAT(*const_(o), Eq(0));
    EXPECT_THAT(o.value_or(1), Eq(0));
    EXPECT_THAT(const_(o).value_or(1), Eq(0));

    o.emplace(123);
    EXPECT_THAT(o.has_value(), Eq(true));
    EXPECT_THAT(o.value(), Eq(123));
    EXPECT_THAT(const_(o).value(), Eq(123));
    EXPECT_THAT(*o, Eq(123));
    EXPECT_THAT(*const_(o), Eq(123));
    EXPECT_THAT(o.value_or(1), Eq(123));
    EXPECT_THAT(const_(o).value_or(1), Eq(123));

    o.reset();
    EXPECT_THAT(o.has_value(), Eq(false));
    EXPECT_THROW(o.value(), std::runtime_error);
    EXPECT_THROW(const_(o).value(), std::runtime_error);
    EXPECT_THAT(o.value_or(1), Eq(1));
    EXPECT_THAT(const_(o).value_or(1), Eq(1));
}

TEST_F(OptionalTest, String) {
    optional<pn::string> o;
    EXPECT_THAT(o.has_value(), Eq(false));
    EXPECT_THROW(o.value(), std::runtime_error);
    EXPECT_THROW(const_(o).value(), std::runtime_error);

    o = nullopt;
    EXPECT_THAT(o.has_value(), Eq(false));
    EXPECT_THROW(o.value(), std::runtime_error);
    EXPECT_THROW(const_(o).value(), std::runtime_error);

    o.reset();
    EXPECT_THAT(o.has_value(), Eq(false));
    EXPECT_THROW(o.value(), std::runtime_error);
    EXPECT_THROW(const_(o).value(), std::runtime_error);

    o.emplace();
    EXPECT_THAT(o.has_value(), Eq(true));
    EXPECT_THAT(o.value(), Eq<pn::string_view>(""));
    EXPECT_THAT(const_(o).value(), Eq<pn::string_view>(""));
    EXPECT_THAT(*o, Eq<pn::string_view>(""));
    EXPECT_THAT(*const_(o), Eq<pn::string_view>(""));
    EXPECT_THAT(o->substr(0), Eq<pn::string_view>(""));
    EXPECT_THAT(const_(o)->substr(0), Eq<pn::string_view>(""));

    o.emplace("123");
    EXPECT_THAT(o.has_value(), Eq(true));
    EXPECT_THAT(o.value(), Eq<pn::string_view>("123"));
    EXPECT_THAT(const_(o).value(), Eq<pn::string_view>("123"));
    EXPECT_THAT(*o, Eq<pn::string_view>("123"));
    EXPECT_THAT(*const_(o), Eq<pn::string_view>("123"));
    EXPECT_THAT(o->substr(0), Eq<pn::string_view>("123"));
    EXPECT_THAT(const_(o)->substr(0), Eq<pn::string_view>("123"));

    o.emplace("zzzz", 3);
    EXPECT_THAT(o.has_value(), Eq(true));
    EXPECT_THAT(o.value(), Eq<pn::string_view>("zzz"));
    EXPECT_THAT(const_(o).value(), Eq<pn::string_view>("zzz"));
    EXPECT_THAT(*o, Eq<pn::string_view>("zzz"));
    EXPECT_THAT(*const_(o), Eq<pn::string_view>("zzz"));
    EXPECT_THAT(o->substr(0), Eq<pn::string_view>("zzz"));
    EXPECT_THAT(const_(o)->substr(0), Eq<pn::string_view>("zzz"));

    o.reset();
    EXPECT_THAT(o.has_value(), Eq(false));
    EXPECT_THROW(o.value(), std::runtime_error);
    EXPECT_THROW(const_(o).value(), std::runtime_error);
}

TEST_F(OptionalTest, CopyAssign) {
    // Test copy constructor from both !has_value() and has_value().
    optional<int> o;
    optional<int> o2(o);  // from !has_value().
    EXPECT_THAT(o2.has_value(), Eq(false));

    optional<int> o3;
    o3.emplace(123);
    optional<int> o4(o3);  // From has_value().
    EXPECT_THAT(*o4, Eq(123));

    *o4 = 456;

    // Test all edges in assignment from optional<T>:
    o = o2;  // !has_value() to !has_value().
    EXPECT_THAT(o.has_value(), Eq(false));
    o = o3;  // !has_value() to has_value().
    EXPECT_THAT(*o, Eq(123));
    o = o4;  // has_value() to has_value().
    EXPECT_THAT(*o, Eq(456));
    o = o2;  // has_value() to !has_value().
    EXPECT_THAT(o.has_value(), Eq(false));

    // Copy from non-optional<T>:
    o.reset();
    o = make_optional(123);  // !has_value() to has_value().
    EXPECT_THAT(*o, Eq(123));
    o = make_optional(456);  // has_value() to has_value().
    EXPECT_THAT(*o, Eq(456));
}

}  // namespace
}  // namespace sfz
