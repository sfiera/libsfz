// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Io.hpp"

#include "sfz/Bytes.hpp"
#include "sfz/Encoding.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Format.hpp"
#include "sfz/PosixFormatter.hpp"

using sfz::Bytes;
using sfz::Rune;
using sfz::String;
using sfz::StringPiece;

namespace utf8 = sfz::utf8;

namespace sfz {
namespace io {

namespace {

Io fds[] = {{0}, {1}, {2}};

}  // namespace

void Io::append(const char* string) {
    append(StringPiece(string));
}

void Io::append(String& string) {
    append(StringPiece(string));
}

void Io::append(const StringPiece& string) {
    Bytes bytes(utf8::encode(string));
    BytesPiece remainder(bytes);
    while (!remainder.empty()) {
        ssize_t result = ::write(fd, remainder.data(), remainder.size());
        if (result < 0) {
            throw Exception(format("write: {0}", posix_strerror()));
        } else {
            remainder.shift(result);
        }
    }
}

void Io::append(size_t num, Rune rune) {
    String string(num, rune);
    append(StringPiece(string));
}

Io* in = &fds[0];
Io* out = &fds[1];
Io* err = &fds[2];

}  // namespace io
}  // namespace sfz
