// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/StringUtilities.hpp"

#include "sfz/CString.hpp"

namespace sfz {

bool string_to_int32_t(const StringPiece& s, int32_t* out, int base) {
    char* end;
    CString c_str(s);
    int32_t id = strtoul(c_str.data(), &end, base);
    if (end != (c_str.data() + c_str.size())) {
        return false;
    }
    *out = id;
    return true;
}

}  // namespace sfz
