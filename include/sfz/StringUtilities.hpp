// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_STRING_UTILITIES_HPP_
#define SFZ_STRING_UTILITIES_HPP_

#include <stdint.h>
#include "sfz/String.hpp"

namespace sfz {

bool string_to_int32_t(const StringPiece& s, int32_t* out, int base = 10);

}  // namespace sfz

#endif  // SFZ_STRING_UTILITIES_HPP_
