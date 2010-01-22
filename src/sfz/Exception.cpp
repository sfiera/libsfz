// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Exception.hpp"

#include "sfz/Format.hpp"

namespace sfz {

Exception::Exception(const char* fmt, SFZ_FORMAT_ITEMS_DEFINITION) {
    const FormatItem* items[SFZ_FORMAT_ITEM_COUNT] = SFZ_FORMAT_ITEMS_ARRAY;
    FormatResult(fmt, SFZ_FORMAT_ITEM_COUNT, items).print_to(&_explanation);
}

Exception::~Exception() throw() { }

void Exception::print_to(String* out) const throw() {
    out->append(_explanation);
}

}  // namespace sfz
