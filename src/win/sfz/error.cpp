// Copyright (c) 2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "win.hpp"

#include <sfz/error.hpp>

namespace sfz {

int win_last_error() { return GetLastError(); }

pn::string win_strerror(int error) {
    if (error == 0) {
        error = win_last_error();
    }
    LPVOID buf;
    int    length;
    if ((length = FormatMessage(
                 FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 reinterpret_cast<LPTSTR>(&buf), 0, NULL)) == 0) {
        return "unknown error";
    }
    pn::string result{reinterpret_cast<const char*>(buf), length};
    LocalFree(buf);
    return result;
}

int posix_errno() { return errno; }

pn::string posix_strerror(int error) {
    if (error == 0) {
        error = errno;
    }
    static const int buf_size = 256;
    char             buf[buf_size];
    strerror_s(buf, buf_size, error);
    return pn::string_view{buf}.copy();
}

}  // namespace sfz
