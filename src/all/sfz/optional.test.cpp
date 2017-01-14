// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/optional.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/string.hpp>

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
    Optional<int> o;
    EXPECT_THAT(o.has(), Eq(false));
    EXPECT_THAT(o.get(), Eq<int*>(NULL));
    EXPECT_THAT(const_(o).get(), Eq<int*>(NULL));
    EXPECT_THROW(*o, Exception);
    EXPECT_THROW(*const_(o), Exception);

    o.clear();
    EXPECT_THAT(o.has(), Eq(false));
    EXPECT_THAT(o.get(), Eq<int*>(NULL));
    EXPECT_THAT(const_(o).get(), Eq<int*>(NULL));
    EXPECT_THROW(*o, Exception);
    EXPECT_THROW(*const_(o), Exception);

    o.set();
    EXPECT_THAT(o.has(), Eq(true));
    EXPECT_THAT(*o.get(), Eq(0));
    EXPECT_THAT(*const_(o).get(), Eq(0));
    EXPECT_THAT(*o, Eq(0));
    EXPECT_THAT(*const_(o), Eq(0));

    o.set(123);
    EXPECT_THAT(o.has(), Eq(true));
    EXPECT_THAT(*o.get(), Eq(123));
    EXPECT_THAT(*const_(o).get(), Eq(123));
    EXPECT_THAT(*o, Eq(123));
    EXPECT_THAT(*const_(o), Eq(123));

    o.clear();
    EXPECT_THAT(o.has(), Eq(false));
    EXPECT_THAT(o.get(), Eq<int*>(NULL));
    EXPECT_THAT(const_(o).get(), Eq<int*>(NULL));
    EXPECT_THROW(*o, Exception);
    EXPECT_THROW(*const_(o), Exception);
}

TEST_F(OptionalTest, String) {
    Optional<String> o;
    EXPECT_THAT(o.has(), Eq(false));
    EXPECT_THAT(o.get(), Eq<String*>(NULL));
    EXPECT_THAT(const_(o).get(), Eq<String*>(NULL));
    EXPECT_THROW(*o, Exception);
    EXPECT_THROW(*const_(o), Exception);
    EXPECT_THROW(o->slice(), Exception);
    EXPECT_THROW(const_(o)->slice(), Exception);

    o.clear();
    EXPECT_THAT(o.has(), Eq(false));
    EXPECT_THAT(o.get(), Eq<String*>(NULL));
    EXPECT_THAT(const_(o).get(), Eq<String*>(NULL));
    EXPECT_THROW(*o, Exception);
    EXPECT_THROW(*const_(o), Exception);
    EXPECT_THROW(o->slice(), Exception);
    EXPECT_THROW(const_(o)->slice(), Exception);

    o.set();
    EXPECT_THAT(o.has(), Eq(true));
    EXPECT_THAT(*o.get(), Eq<StringSlice>(""));
    EXPECT_THAT(*const_(o).get(), Eq<StringSlice>(""));
    EXPECT_THAT(*o, Eq<StringSlice>(""));
    EXPECT_THAT(*const_(o), Eq<StringSlice>(""));
    EXPECT_THAT(o->slice(), Eq<StringSlice>(""));
    EXPECT_THAT(const_(o)->slice(), Eq<StringSlice>(""));

    o.set("123");
    EXPECT_THAT(o.has(), Eq(true));
    EXPECT_THAT(*o.get(), Eq<StringSlice>("123"));
    EXPECT_THAT(*const_(o).get(), Eq<StringSlice>("123"));
    EXPECT_THAT(*o, Eq<StringSlice>("123"));
    EXPECT_THAT(*const_(o), Eq<StringSlice>("123"));
    EXPECT_THAT(o->slice(), Eq<StringSlice>("123"));
    EXPECT_THAT(const_(o)->slice(), Eq<StringSlice>("123"));

    o.set(3, 'z');
    EXPECT_THAT(o.has(), Eq(true));
    EXPECT_THAT(*o.get(), Eq<StringSlice>("zzz"));
    EXPECT_THAT(*const_(o).get(), Eq<StringSlice>("zzz"));
    EXPECT_THAT(*o, Eq<StringSlice>("zzz"));
    EXPECT_THAT(*const_(o), Eq<StringSlice>("zzz"));
    EXPECT_THAT(o->slice(), Eq<StringSlice>("zzz"));
    EXPECT_THAT(const_(o)->slice(), Eq<StringSlice>("zzz"));

    o.clear();
    EXPECT_THAT(o.has(), Eq(false));
    EXPECT_THAT(o.get(), Eq<String*>(NULL));
    EXPECT_THAT(const_(o).get(), Eq<String*>(NULL));
    EXPECT_THROW(*o, Exception);
    EXPECT_THROW(*const_(o), Exception);
    EXPECT_THROW(o->slice(), Exception);
    EXPECT_THROW(const_(o)->slice(), Exception);
}

TEST_F(OptionalTest, CopyAssign) {
    // Test copy constructor from both !has() and has().
    Optional<int> o;
    Optional<int> o2(o);  // from !has().
    EXPECT_THAT(o2.has(), Eq(false));

    Optional<int> o3;
    o3.set(123);
    Optional<int> o4(o3);  // From has().
    EXPECT_THAT(*o4, Eq(123));

    *o4 = 456;

    // Test all edges in assignment from Optional<T>:
    o = o2;  // !has() to !has().
    EXPECT_THAT(o.has(), Eq(false));
    o = o3;  // !has() to has().
    EXPECT_THAT(*o, Eq(123));
    o = o4;  // has() to has().
    EXPECT_THAT(*o, Eq(456));
    o = o2;  // has() to !has().
    EXPECT_THAT(o.has(), Eq(false));

    // Copy from non-Optional<T>:
    o.clear();
    copy(o, 123);  // !has() to has().
    EXPECT_THAT(*o, Eq(123));
    copy(o, 456);  // has() to has().
    EXPECT_THAT(*o, Eq(456));

    // Test all edges in copying from Optional<T>:
    o.clear();
    copy(o, o2);  // !has() to !has().
    EXPECT_THAT(o.has(), Eq(false));
    copy(o, o3);  // !has() to has().
    EXPECT_THAT(*o, Eq(123));
    copy(o, o4);  // has() to has().
    EXPECT_THAT(*o, Eq(456));
    copy(o, o2);  // has() to !has().
    EXPECT_THAT(o.has(), Eq(false));
}

}  // namespace
}  // namespace sfz
