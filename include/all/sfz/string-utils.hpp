// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_STRING_UTILS_HPP_
#define SFZ_STRING_UTILS_HPP_

#include <stdint.h>
#include <pn/fwd>

namespace sfz {

pn::string upper(pn::string_view s);
pn::string lower(pn::string_view s);

}  // namespace sfz

#endif  // SFZ_STRING_UTILS_HPP_
