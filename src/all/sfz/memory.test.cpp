// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/memory.hpp>

#include <algorithm>
#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/exception.hpp>
#include <sfz/foreach.hpp>

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
            foreach (int index, sequence) {
                ptrs[index]->mark();
                ptrs[index].reset();
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

// Test swap(linked_ptr& x, linked_ptr& y).
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
    EXPECT_THROW(swap(one, two), Exception);
    EXPECT_THROW(swap(two, one), Exception);
    block.reset();
    swap(one, two);
}

// Test linked_array<>.
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

}  // namespace
}  // namespace sfz
