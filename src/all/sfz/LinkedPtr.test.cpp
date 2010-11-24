// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/LinkedPtr.hpp"

#include <algorithm>
#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "sfz/Exception.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"

using std::next_permutation;
using std::set;
using std::vector;
using testing::Contains;
using testing::Eq;
using testing::Ge;
using testing::Gt;
using testing::InSequence;
using testing::Le;
using testing::Lt;
using testing::Ne;
using testing::Not;
using testing::Test;

namespace sfz {
namespace {

typedef Test LinkedPtrTest;

class LifeObserver {
  public:
    LifeObserver() { }

    MOCK_CONST_METHOD0(create, void());
    MOCK_CONST_METHOD0(mark, void());
    MOCK_CONST_METHOD0(destroy, void());

  private:
    DISALLOW_COPY_AND_ASSIGN(LifeObserver);
};

struct ObservedObject {
    const LifeObserver* observer;

    ObservedObject()
            : observer(NULL) { }

    ObservedObject(const LifeObserver* observer)
            : observer(observer) {
        observer->create();
    }

    ~ObservedObject() { observer->destroy(); }

    void mark() { observer->mark(); }

    DISALLOW_COPY_AND_ASSIGN(ObservedObject);
};

class DerivedObject : public ObservedObject {
  public:
    DerivedObject(const LifeObserver* observer)
            : ObservedObject(observer) { }

    DISALLOW_COPY_AND_ASSIGN(DerivedObject);
};

TEST_F(LinkedPtrTest, Singular) {
    LifeObserver observer;
    {
        InSequence s;
        EXPECT_CALL(observer, create());
        EXPECT_CALL(observer, mark());
        EXPECT_CALL(observer, destroy());
    }

    linked_ptr<ObservedObject> obj(new ObservedObject(&observer));
    obj->mark();
}

// Test with three linked_ptr<> objects in the same group.  For completeness, we consider both
// possible assignments ("B = A, C = A", and "B = A, C = B"), and all six orders in which the
// objects could be released, by using next_permutation().
TEST_F(LinkedPtrTest, Triple) {
    for (int i = 0; i < 2; ++i) {
        vector<int> sequence;
        sequence.push_back(0);
        sequence.push_back(1);
        sequence.push_back(2);
        do {
            LifeObserver observer;
            {
                InSequence s;
                EXPECT_CALL(observer, create());
                EXPECT_CALL(observer, mark()).Times(3);
                EXPECT_CALL(observer, destroy());
            }

            linked_ptr<ObservedObject> ptrs[3];
            ptrs[0].reset(new ObservedObject(&observer));
            ptrs[1] = ptrs[0];
            ptrs[2] = ptrs[i];
            foreach (it, sequence) {
                ptrs[*it]->mark();
                ptrs[*it].reset();
            }
        } while (next_permutation(sequence.begin(), sequence.end()));
    }
}

// Test that the copy constructor allows a linked_ptr<> to be created from a heterogenous pointer
// type, so long as the pointer types are assignment-compatible.
TEST_F(LinkedPtrTest, HeterogeneousCopy) {
    LifeObserver observer;
    {
        InSequence s;
        EXPECT_CALL(observer, create());
        EXPECT_CALL(observer, destroy());
    }
    linked_ptr<DerivedObject> derived(new DerivedObject(&observer));
    linked_ptr<ObservedObject> base(derived);
}

// Test that the assignment operator allows a linked_ptr<> to be assigned the value of a
// heterogenous pointer type, so long as the pointer types are assignment-compatible.  This is
// mostly a test that the code compiles, rather than that it is correct.
TEST_F(LinkedPtrTest, HeterogeneousAssign) {
    LifeObserver observer;
    {
        InSequence s;
        EXPECT_CALL(observer, create());
        EXPECT_CALL(observer, destroy());
    }
    linked_ptr<ObservedObject> base;
    linked_ptr<DerivedObject> derived(new DerivedObject(&observer));
    base = derived;
}

// Test linked_ptr<>::release().  When there are multiple linked_ptr<> objects in the group,
// release() should be disallowed and throw an exception.  When there is a single object in the
// group, release should release ownership and return it.
TEST_F(LinkedPtrTest, Release) {
    LifeObserver observer;
    {
        InSequence s;
        EXPECT_CALL(observer, create());
        EXPECT_CALL(observer, mark()).Times(2);
        EXPECT_CALL(observer, destroy());
    }
    linked_ptr<ObservedObject> one(new ObservedObject(&observer));
    linked_ptr<ObservedObject> two(one);
    EXPECT_THROW(one.release(), Exception);
    one.reset();
    two->mark();
    one.reset(two.release());
    one->mark();
}

// Test linked_ptr<>::swap().
TEST_F(LinkedPtrTest, Swap) {
    LifeObserver observer_one;
    LifeObserver observer_two;
    {
        InSequence s;
        EXPECT_CALL(observer_one, create());
        EXPECT_CALL(observer_two, create());
        EXPECT_CALL(observer_one, destroy());
        EXPECT_CALL(observer_two, destroy());
    }
    linked_ptr<ObservedObject> one(new ObservedObject(&observer_one));
    linked_ptr<ObservedObject> two(new ObservedObject(&observer_two));
    linked_ptr<ObservedObject> block = two;
    EXPECT_THROW(one.swap(&two), Exception);
    EXPECT_THROW(two.swap(&one), Exception);
    block.reset();
    one.swap(&two);
}

// Test linked_ptr<>::swap().
TEST_F(LinkedPtrTest, LinkedArray) {
    LifeObserver observer;
    {
        InSequence s;
        EXPECT_CALL(observer, destroy()).Times(3);
    }
    linked_array<ObservedObject> array(new ObservedObject[3]);
    array[0].observer = &observer;
    array[1].observer = &observer;
    array[2].observer = &observer;
}

TEST_F(LinkedPtrTest, LinkedKey) {
    linked_key<int> san(new int(3));
    linked_key<int> yon(new int(4));
    linked_key<int> shi(new int(4));

    EXPECT_THAT(yon, Eq(shi));
    EXPECT_THAT(san, Ne(yon));
    EXPECT_THAT(san, Lt(yon));
    EXPECT_THAT(yon, Le(shi));
    EXPECT_THAT(yon, Gt(san));
    EXPECT_THAT(yon, Ge(shi));

    EXPECT_THAT(san, Not(Eq(yon)));
    EXPECT_THAT(yon, Not(Ne(shi)));
    EXPECT_THAT(yon, Not(Lt(shi)));
    EXPECT_THAT(yon, Not(Le(san)));
    EXPECT_THAT(yon, Not(Gt(shi)));
    EXPECT_THAT(san, Not(Ge(yon)));
}

TEST_F(LinkedPtrTest, LinkedKeyIntegerSet) {
    set<linked_key<int> > s;

    s.insert(make_linked_key(new int(1)));
    s.insert(make_linked_key(new int(3)));
    s.insert(make_linked_key(new int(5)));
    EXPECT_THAT(s, Not(Contains(make_linked_key(new int(0)))));
    EXPECT_THAT(s, Contains(make_linked_key(new int(1))));
    EXPECT_THAT(s, Not(Contains(make_linked_key(new int(2)))));
    EXPECT_THAT(s, Contains(make_linked_key(new int(3))));
    EXPECT_THAT(s, Not(Contains(make_linked_key(new int(4)))));
    EXPECT_THAT(s, Contains(make_linked_key(new int(5))));
    EXPECT_THAT(s, Not(Contains(make_linked_key(new int(6)))));

    s.erase(make_linked_key(new int(3)));
    EXPECT_THAT(s, Not(Contains(make_linked_key(new int(3)))));
}

class ObservedKey : public ObservedObject {
  public:
    ObservedKey(const LifeObserver* observer, int value)
            : ObservedObject(observer),
              _value(value) { }

    int value() const { return _value; }

  private:
    const int _value;

    DISALLOW_COPY_AND_ASSIGN(ObservedKey);
};
bool operator==(const ObservedKey& lhs, const ObservedKey& rhs) {
    return lhs.value() == rhs.value();
}
bool operator<(const ObservedKey& lhs, const ObservedKey& rhs) {
    return lhs.value() < rhs.value();
}

TEST_F(LinkedPtrTest, LinkedKeyObservedSet) {
    LifeObserver observer;
    {
        InSequence s;
        EXPECT_CALL(observer, create()).Times(7);
        EXPECT_CALL(observer, destroy()).Times(7);
    }

    set<linked_key<ObservedKey> > s;
    s.insert(make_linked_key(new ObservedKey(&observer, 1)));
    s.insert(make_linked_key(new ObservedKey(&observer, 3)));

    linked_key<ObservedKey> k0(new ObservedKey(&observer, 0));
    linked_key<ObservedKey> k1(new ObservedKey(&observer, 1));
    linked_key<ObservedKey> k2(new ObservedKey(&observer, 2));
    linked_key<ObservedKey> k3(new ObservedKey(&observer, 3));
    linked_key<ObservedKey> k4(new ObservedKey(&observer, 4));
    EXPECT_THAT(s, Not(Contains(k0)));
    EXPECT_THAT(s, Contains(k1));
    EXPECT_THAT(s, Not(Contains(k2)));
    EXPECT_THAT(s, Contains(k3));
    EXPECT_THAT(s, Not(Contains(k4)));

    s.erase(k3);
    EXPECT_THAT(s, Not(Contains(k3)));
}

}  // namespace
}  // namespace sfz
