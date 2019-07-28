// Copyright (c) 2009-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/error.hpp>

#include <errno.h>
#include <string.h>
#include <pn/output>
#include <sfz/encoding.hpp>

namespace sfz {

int posix_errno() { return errno; }

pn::string posix_strerror(int error) {
    if (error == 0) {
        error = errno;
    }
    static const int buf_size = 256;
    char             buf[buf_size];
#ifdef _GNU_SOURCE
    const char* msg = strerror_r(error, buf, buf_size);
    return pn::string_view{msg}.copy();
#else
    int result = strerror_r(error, buf, buf_size);
    static_cast<void>(result);
    return pn::string_view{buf}.copy();
#endif
}

}  // namespace sfz
