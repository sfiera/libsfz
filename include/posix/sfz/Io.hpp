// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_IO_HPP_
#define SFZ_IO_HPP_

#include <sfz/String.hpp>
#include <sfz/print.hpp>

namespace sfz {
namespace io {

struct Io {
    int fd;
    void push(const StringSlice& string);
    void push(size_t num, Rune rune);
};

extern Io* in;
extern Io* out;
extern Io* err;

}  // namespace  io
}  // namespace sfz

#endif  // SFZ_IO_HPP_
