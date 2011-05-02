// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/ref-count.hpp>

#include <stdio.h>
#include <stdlib.h>

namespace sfz {

ReferenceCounted::ReferenceCounted()
    : _reference_count(0) { }

ReferenceCounted::~ReferenceCounted() {
    int32_t count = _reference_count;
    if (count != 0) {
        fprintf(stderr, "~ReferenceCounted() with _reference_count == %d\n", count);
        abort();
    }
}

void ReferenceCounted::ref() const {
    __sync_add_and_fetch(&_reference_count, 1);
}

bool ReferenceCounted::unref() const {
    if (__sync_add_and_fetch(&_reference_count, -1) == 0) {
        delete this;
        return true;
    }
    return false;
}

}  // namespace sfz
