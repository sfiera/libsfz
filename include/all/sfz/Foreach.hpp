// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FOREACH_HPP_
#define SFZ_FOREACH_HPP_

#   if (_MSC_VER >= 1600) && (_MSC_VER < 1700)
#       define SFZ_DECLTYPE(x) decltype(x)
#   elif __GNUC__ == 4
#       define SFZ_DECLTYPE(x) __typeof__(x)
#   else
#       define SFZ_DECLTYPE(x) int
#       error "Don't know how to get decltype-like functionality"
#   endif

#define foreach(VAR, CONTAINER) \
    for (bool __loop = true; __loop; ) \
    for (const SFZ_DECLTYPE(CONTAINER)& __container = (CONTAINER); __loop; __loop = false) \
    for (SFZ_DECLTYPE(::sfz::begin(__container)) VAR = ::sfz::begin(__container), \
            __end = ::sfz::end(__container); VAR != __end; ++VAR)

namespace sfz {

template <typename T>
typename T::const_iterator begin(const T& container) {
    return container.begin();
}

template <typename T, int size>
const T* begin(const T (&array)[size]) {
    return array;
}

template <typename T>
typename T::const_iterator end(const T& container) {
    return container.end();
}

template <typename T, int size>
const T* end(const T (&array)[size]) {
    return array + size;
}

}  // namespace sfz

#endif  // SFZ_FOREACH_HPP_
