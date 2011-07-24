// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_ALGORITHM_HPP_
#define SFZ_ALGORITHM_HPP_

namespace sfz {

template <typename T>
void copy(T& to, const T& from) {
    to = from;
}

}  // namespace sfz

#endif  // SFZ_ALGORITHM_HPP_
