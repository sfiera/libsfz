// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Exception.hpp"

#include "sfz/Format.hpp"

namespace sfz {

Exception::Exception(const char* fmt, SFZ_FORMAT_ITEMS_DEFINITION) {
    const PrintItem* items[SFZ_FORMAT_ITEM_COUNT] = SFZ_FORMAT_ITEMS_ARRAY;
    FormatResult(fmt, SFZ_FORMAT_ITEM_COUNT, items).print_to(&_explanation);
    _utf8_string.assign(_explanation, utf8_encoding());
    _utf8_string.resize(_utf8_string.size() + 1);
}

Exception::~Exception() throw() { }

void Exception::print_to(String* out) const throw() {
    out->append(_explanation);
}

const char* Exception::what() const throw() {
    return reinterpret_cast<const char*>(_utf8_string.data());
}

}  // namespace sfz
