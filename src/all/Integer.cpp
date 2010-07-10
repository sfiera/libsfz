// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Integer.hpp"

namespace sfz {

Integer::Integer(signed char value)
    : _negative(value < 0),
      _abs(_negative ? -static_cast<int64_t>(value) : value) { }

Integer::Integer(signed short value)
    : _negative(value < 0),
      _abs(_negative ? -static_cast<int64_t>(value) : value) { }

Integer::Integer(signed int value)
    : _negative(value < 0),
      _abs(_negative ? -static_cast<int64_t>(value) : value) { }

Integer::Integer(signed long value)
    : _negative(value < 0),
      _abs(_negative ? -static_cast<int64_t>(value) : value) { }

Integer::Integer(signed long long value)
    : _negative(value < 0),
      _abs(_negative ? -static_cast<int64_t>(value) : value) { }

Integer::Integer(unsigned char value)
    : _negative(false),
      _abs(value) { }

Integer::Integer(unsigned short value)
    : _negative(false),
      _abs(value) { }

Integer::Integer(unsigned int value)
    : _negative(false),
      _abs(value) { }

Integer::Integer(unsigned long value)
    : _negative(false),
      _abs(value) { }

Integer::Integer(unsigned long long value)
    : _negative(false),
      _abs(value) { }

}  // namespace sfz
