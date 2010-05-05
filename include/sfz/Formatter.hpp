// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMATTER_HPP_
#define SFZ_FORMATTER_HPP_

#include "sfz/PrintItem.hpp"

#include "sfz/String.hpp"

namespace sfz {

template <typename T>
PrintItem dec(T value, size_t min_width = 1) {
    return PrintItem(value, 10, min_width);
}

template <typename T>
PrintItem hex(T value, size_t min_width = 1) {
    return PrintItem(value, 16, min_width);
}

template <typename T>
PrintItem oct(T value, size_t min_width = 1) {
    return PrintItem(value, 8, min_width);
}

template <typename T>
PrintItem bin(T value, size_t min_width = 1) {
    return PrintItem(value, 2, min_width);
}

PrintItem escape(const StringPiece& string);
PrintItem quote(const StringPiece& string);

}  // namespace sfz

#endif  // SFZ_FORMATTER_HPP_
