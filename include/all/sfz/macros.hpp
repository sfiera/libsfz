// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_MACROS_HPP_
#define SFZ_MACROS_HPP_

#define DISALLOW_COPY_AND_ASSIGN(CLASS) \
  private: \
    CLASS(const CLASS&); \
    CLASS& operator=(const CLASS&);

#define SFZ_OPERATORS_BASED_ON_COMPARE_T(TEMPLATE, TYPE) \
    TEMPLATE bool operator==(const TYPE& x, const TYPE& y) { return compare(x, y) == 0; } \
    TEMPLATE bool operator!=(const TYPE& x, const TYPE& y) { return compare(x, y) != 0; } \
    TEMPLATE bool operator< (const TYPE& x, const TYPE& y) { return compare(x, y) <  0; } \
    TEMPLATE bool operator<=(const TYPE& x, const TYPE& y) { return compare(x, y) <= 0; } \
    TEMPLATE bool operator> (const TYPE& x, const TYPE& y) { return compare(x, y) >  0; } \
    TEMPLATE bool operator>=(const TYPE& x, const TYPE& y) { return compare(x, y) >= 0; }

#define SFZ_OPERATORS_BASED_ON_COMPARE(TYPE) \
    SFZ_OPERATORS_BASED_ON_COMPARE_T(/* not templated */, TYPE)

#endif  // SFZ_MACROS_HPP_
