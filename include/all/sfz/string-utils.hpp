// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_STRING_UTILS_HPP_
#define SFZ_STRING_UTILS_HPP_

#include <stdint.h>
#include <sfz/bytes.hpp>
#include <sfz/macros.hpp>
#include <sfz/string.hpp>

namespace sfz {

class StringSlice;

template <typename T>
bool string_to_int(StringSlice s, T& out, int base = 10);

template <typename T>
bool string_to_float(StringSlice s, T& out);

class CString {
  public:
    CString(const StringSlice& string);

    char* data();
    const char* data() const;
    size_t size() const;

  private:
    Bytes _bytes;

    DISALLOW_COPY_AND_ASSIGN(CString);
};

}  // namespace sfz

#endif  // SFZ_STRING_UTILS_HPP_
