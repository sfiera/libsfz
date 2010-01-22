// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMATTER_HPP_
#define SFZ_FORMATTER_HPP_

#include "sfz/Format.hpp"

#include "sfz/String.hpp"

namespace sfz {

template <typename T>
FormatItem dec(T value, size_t min_width = 1) {
    return FormatItem(value, 10, min_width);
}

template <typename T>
FormatItem hex(T value, size_t min_width = 1) {
    return FormatItem(value, 16, min_width);
}

template <typename T>
FormatItem oct(T value, size_t min_width = 1) {
    return FormatItem(value, 8, min_width);
}

template <typename T>
FormatItem bin(T value, size_t min_width = 1) {
    return FormatItem(value, 2, min_width);
}

FormatItem escape(const StringPiece& string);
FormatItem quote(const StringPiece& string);

}  // namespace sfz

#endif  // SFZ_FORMATTER_HPP_
