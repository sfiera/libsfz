// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/ScopedFd.hpp"

#include <unistd.h>
#include "sfz/Exception.hpp"
#include "sfz/PosixFormatter.hpp"
#include "sfz/foreach.hpp"
#include "sfz/format.hpp"

namespace sfz {

ScopedFd::ScopedFd(int fd)
        : _fd(fd) { }

ScopedFd::~ScopedFd() {
    close(_fd);
}

int ScopedFd::get() {
    return _fd;
}

int ScopedFd::release() {
    int fd = _fd;
    _fd = -1;
    return fd;
}

void ScopedFd::push(const BytesSlice& bytes) {
    BytesSlice remainder(bytes);
    while (!remainder.empty()) {
        ssize_t written = ::write(_fd, remainder.data(), remainder.size());
        if (written < 0) {
            throw Exception(format("write: {0}", posix_strerror()));
        } else if (written == 0) {
            throw Exception("write: didn't write anything");
        } else {
            remainder.shift(written);
        }
    }
}

void ScopedFd::push(size_t num, uint8_t byte) {
    foreach (int i, range(num)) {
        push(BytesSlice(&byte, 1));
    }
}

}  // namespace sfz
