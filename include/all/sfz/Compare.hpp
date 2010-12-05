// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_COMPARE_HPP_
#define SFZ_COMPARE_HPP_

namespace sfz {

template <typename T>
int compare(const T& x, const T& y) {
    if (x < y) {
        return -1;
    } else if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

#define SFZ_OPERATORS_BASED_ON_COMPARE_T(TEMPLATE, TYPE) \
    TEMPLATE bool operator==(const TYPE& x, const TYPE& y) { return compare(x, y) == 0; } \
    TEMPLATE bool operator!=(const TYPE& x, const TYPE& y) { return compare(x, y) != 0; } \
    TEMPLATE bool operator< (const TYPE& x, const TYPE& y) { return compare(x, y) <  0; } \
    TEMPLATE bool operator<=(const TYPE& x, const TYPE& y) { return compare(x, y) <= 0; } \
    TEMPLATE bool operator> (const TYPE& x, const TYPE& y) { return compare(x, y) >  0; } \
    TEMPLATE bool operator>=(const TYPE& x, const TYPE& y) { return compare(x, y) >= 0; }

#define SFZ_OPERATORS_BASED_ON_COMPARE(TYPE) \
    SFZ_OPERATORS_BASED_ON_COMPARE_T(/* not templated */, TYPE)

}  // namespace sfz

#endif  // SFZ_COMPARE_HPP_
