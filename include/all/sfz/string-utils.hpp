// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_STRING_UTILS_HPP_
#define SFZ_STRING_UTILS_HPP_

#include <stdint.h>
#include <pn/fwd>
#include <sfz/bytes.hpp>
#include <sfz/macros.hpp>

namespace sfz {

class String;
class StringSlice;

void upper(String& s);
void lower(String& s);

pn::string upper(pn::string_view s);
pn::string lower(pn::string_view s);

class CString {
  public:
    CString(const StringSlice& string);

    char*       data();
    const char* data() const;
    size_t      size() const;

  private:
    Bytes _bytes;

    DISALLOW_COPY_AND_ASSIGN(CString);
};

}  // namespace sfz

#endif  // SFZ_STRING_UTILS_HPP_
