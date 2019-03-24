// Copyright (c) 2009-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/string-utils.hpp>

#include <wctype.h>
#include <limits>
#include <pn/string>
#include <sfz/encoding.hpp>
#include <stdexcept>

using std::numeric_limits;

namespace sfz {

pn::string upper(pn::string_view s) {
    pn::string out;
    for (pn::rune r : s) {
        out += pn::rune(towupper(r.value()));
    }
    return out;
}

pn::string lower(pn::string_view s) {
    pn::string out;
    for (pn::rune r : s) {
        out += pn::rune(towlower(r.value()));
    }
    return out;
}

}  // namespace sfz
