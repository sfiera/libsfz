// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/error.hpp>

#include <errno.h>
#include <string.h>
#include <sfz/encoding.hpp>

namespace sfz {

int posix_errno() { return errno; }

pn::string posix_strerror(int error) {
    if (error == 0) {
        error = errno;
    }
    static const int buf_size = 256;
    char             buf[buf_size];
    strerror_r(error, buf, buf_size);
    return pn::string_view{buf}.copy();
}

}  // namespace sfz
