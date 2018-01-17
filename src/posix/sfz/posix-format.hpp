// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_POSIX_FORMAT_HPP_
#define SFZ_POSIX_FORMAT_HPP_

#include <pn/string>
#include <sfz/print.hpp>

namespace sfz {

int posix_errno();

struct PosixStrerror;
PosixStrerror posix_strerror(int error = 0);

struct PosixStrerror {
    PosixStrerror(int e) : error{e} {}
    pn::string string();
    int        error;
};
void print_to(PrintTarget out, const PosixStrerror& error);

}  // namespace sfz

#endif  // SFZ_POSIX_FORMAT_HPP_
