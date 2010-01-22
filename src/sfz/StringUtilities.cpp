// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/StringUtilities.hpp"

#include "sfz/Bytes.hpp"
#include "sfz/Format.hpp"
#include "sfz/Formatter.hpp"

namespace sfz {

bool string_to_int32_t(const StringPiece& s, int32_t* out, int base) {
    char* end;
    Bytes encoded(s, ascii_encoding());
    encoded.resize(encoded.size() + 1);
    const char* c_str = reinterpret_cast<const char*>(encoded.data());
    int32_t id = strtoul(c_str, &end, base);
    if (end != (c_str + strlen(c_str))) {
        return false;
    }
    *out = id;
    return true;
}

}  // namespace sfz
