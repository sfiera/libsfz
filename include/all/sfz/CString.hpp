// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_C_STRING_HPP_
#define SFZ_C_STRING_HPP_

#include <sfz/Bytes.hpp>
#include <sfz/Macros.hpp>
#include <sfz/String.hpp>

namespace sfz {

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

#endif  // SFZ_C_STRING_HPP_
