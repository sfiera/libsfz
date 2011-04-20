// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/foreach.hpp>

#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/print.hpp>
#include <sfz/string.hpp>

using std::vector;
using testing::Eq;
using testing::Ne;
using testing::StrictMock;
using testing::Test;

namespace sfz {
namespace {

template <typename T>
class MockFunctor;

template <typename T>
class MockFunctor<void(const T&)> {
  public:
    MOCK_METHOD1_T(Call, void(const T&));
    void operator()(const T& t) { Call(t); }
};

template <typename T>
class MockFunctor<void(T)> : public MockFunctor<void(const T&)> {};

typedef Test ForeachTest;

// Creates a vector of the integers in the range [start, end).
vector<int> mkrange(int start, int end) {
    vector<int> result;
    for (int i = start; i != end; ++i) {
        result.push_back(i);
    }
    return result;
}

// Iteration over an array of primitives.
TEST_F(ForeachTest, IntArray) {
    MockFunctor<void(int)> f;
    EXPECT_CALL(f, Call(0));
    EXPECT_CALL(f, Call(1));
    EXPECT_CALL(f, Call(2));
    EXPECT_CALL(f, Call(3));

    int values[] = {0, 1, 2, 3};
    SFZ_FOREACH(int i, values, {
        f(i);
    });
}

// Iteration over a vector<> of primitives.
TEST_F(ForeachTest, IntVector) {
    MockFunctor<void(int)> f;
    EXPECT_CALL(f, Call(0));
    EXPECT_CALL(f, Call(1));
    EXPECT_CALL(f, Call(2));
    EXPECT_CALL(f, Call(3));

    vector<int> values = mkrange(0, 4);
    SFZ_FOREACH(int i, values, {
        f(i);
    });
}

// As before, except that it is a const vector<>& of a temporary, and not a variable on the stack.
TEST_F(ForeachTest, IntVectorConstRef) {
    MockFunctor<void(int)> f;
    EXPECT_CALL(f, Call(0));
    EXPECT_CALL(f, Call(1));
    EXPECT_CALL(f, Call(2));
    EXPECT_CALL(f, Call(3));

    const vector<int>& values = mkrange(0, 4);
    SFZ_FOREACH(int i, values, {
        f(i);
    });
}

// As before, except that there is no const vector<>& declared (foreach should do so itself).
TEST_F(ForeachTest, IntVectorTemporary) {
    MockFunctor<void(int)> f;
    EXPECT_CALL(f, Call(0));
    EXPECT_CALL(f, Call(1));
    EXPECT_CALL(f, Call(2));
    EXPECT_CALL(f, Call(3));

    SFZ_FOREACH(int i, mkrange(0, 4), {
        f(i);
    });
}

// Array of a non-primitive type.  We would like to test with a vector<String> as well, but String
// cannot be used in a vector<>, by virtue of not allowing implicit copying.
TEST_F(ForeachTest, StringArray) {
    MockFunctor<void(StringSlice)> f;
    EXPECT_CALL(f, Call(Eq("one")));
    EXPECT_CALL(f, Call(Eq("two")));
    EXPECT_CALL(f, Call(Eq("three")));
    EXPECT_CALL(f, Call(Eq("four")));

    String values[4] = {};
    values[0].assign("one");
    values[1].assign("two");
    values[2].assign("three");
    values[3].assign("four");
    SFZ_FOREACH(const String& s, values, {
        f(s);
    });
}

// Break out of a foreach loop prematurely.
TEST_F(ForeachTest, Break) {
    StrictMock<MockFunctor<void(int)> > f;
    EXPECT_CALL(f, Call(Eq(0)));
    EXPECT_CALL(f, Call(Eq(1)));

    int values[] = {0, 1, 2, 3};
    SFZ_FOREACH(int i, values, {
        f(i);
        if (i == 1) {
            break;
        }
    });
}

// Use continue statements from within a foreach loop.
TEST_F(ForeachTest, Continue) {
    StrictMock<MockFunctor<void(int)> > f;
    EXPECT_CALL(f, Call(Eq(1)));
    EXPECT_CALL(f, Call(Eq(3)));

    int values[] = {0, 1, 2, 3};
    SFZ_FOREACH(int i, values, {
        if ((i % 2) == 0) {
            continue;
        }
        f(i);
    });
}

// In addition to being used to foreach with a custom container, the Kingdoms type also differs
// from other containers in that it explicitly zeroes out its storage upon destruction.  The
// purpose of this is to verify that foreach over a temporary correctly persists the storage for
// the duration of the loop.
class Kingdoms {
  public:
    typedef const StringSlice* iterator;
    typedef const StringSlice* const_iterator;

    Kingdoms() {
        values[0] = "Essex";
        values[1] = "Sussex";
        values[2] = "Wessex";
    }

    ~Kingdoms() {
        values[0] = "<deleted>";
        values[1] = "<deleted>";
        values[2] = "<deleted>";
    }

    iterator begin() const { return values; }
    iterator end() const { return values + 3; }

  private:
    StringSlice values[3];
};

// Baseline test: does Kingdoms work at all?
TEST_F(ForeachTest, Kingdoms) {
    MockFunctor<void(StringSlice)> f;
    EXPECT_CALL(f, Call(Eq("Essex")));
    EXPECT_CALL(f, Call(Eq("Sussex")));
    EXPECT_CALL(f, Call(Eq("Wessex")));

    Kingdoms kingdoms;
    SFZ_FOREACH(const StringSlice& s, kingdoms, {
        f(s);
    });
}

// Special test: does foreach extend the lifetime of a temporary?
TEST_F(ForeachTest, KingdomsTemporary) {
    MockFunctor<void(StringSlice)> f;
    EXPECT_CALL(f, Call(Eq("Essex")));
    EXPECT_CALL(f, Call(Eq("Sussex")));
    EXPECT_CALL(f, Call(Eq("Wessex")));

    // Extra parentheses needed to avoid most vexing parse.
    SFZ_FOREACH(const StringSlice& s, (Kingdoms()), {
        f(s);
    });
}

}  // namespace
}  // namespace sfz
