// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/ref-count.hpp>

#include <libkern/OSAtomic.h>
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
    OSAtomicAdd32Barrier(1, &_reference_count);
}

bool ReferenceCounted::unref() const {
    if (OSAtomicAdd32Barrier(-1, &_reference_count) == 0) {
        delete this;
        return true;
    }
    return false;
}

}  // namespace sfz
