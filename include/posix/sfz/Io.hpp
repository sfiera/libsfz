// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_IO_HPP_
#define SFZ_IO_HPP_

#include <sfz/Rune.hpp>
#include <sfz/String.hpp>

namespace sfz {
namespace io {

struct Io {
    int fd;
    void append(const char* string);
    void append(String& string);
    void append(const StringPiece& string);
    void append(size_t num, Rune rune);
};

extern Io* in;
extern Io* out;
extern Io* err;

}  // namespace  io
}  // namespace sfz

#endif  // SFZ_IO_HPP_
