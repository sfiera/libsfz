// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/ScopedFd.hpp"

#include <unistd.h>

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

}  // namespace sfz
