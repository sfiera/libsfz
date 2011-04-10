// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_ALGORITHM_HPP_
#define SFZ_ALGORITHM_HPP_

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

}  // namespace sfz

#endif  // SFZ_ALGORITHM_HPP_
