// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMAT_HPP_
#define SFZ_FORMAT_HPP_

#include <stdint.h>
#include <pn/fwd>

namespace sfz {

class Integer;

pn::string dec(Integer value, int min_width = 1);
pn::string hex(Integer value, int min_width = 1);
pn::string oct(Integer value, int min_width = 1);
pn::string bin(Integer value, int min_width = 1);

// Implementation details follow.

class Integer {
  public:
    Integer(signed char value);
    Integer(signed short value);
    Integer(signed int value);
    Integer(signed long value);
    Integer(signed long long value);

    Integer(unsigned char value);
    Integer(unsigned short value);
    Integer(unsigned int value);
    Integer(unsigned long value);
    Integer(unsigned long long value);

    bool     negative() const { return _negative; }
    uint64_t abs() const { return _abs; }

  private:
    const bool     _negative;
    const uint64_t _abs;
};

}  // namespace sfz

#endif  // SFZ_FORMAT_HPP_
