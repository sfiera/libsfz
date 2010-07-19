// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_SMART_PTR_HPP_
#define SFZ_SMART_PTR_HPP_

namespace sfz {

template <typename T>
struct ptr_traits {
    typedef T   value_type;
    typedef T*  pointer;
    typedef T&  reference;
    static inline void acquire(pointer t) { }
    static inline void release(pointer t) { delete t; }
};

template <typename T>
struct array_traits {
    typedef T   value_type;
    typedef T*  pointer;
    typedef T&  reference;
    static inline void acquire(pointer t) { }
    static inline void release(pointer t) { delete[] t; }
};

template <typename T>
struct ref_traits {
    typedef T   value_type;
    typedef T*  pointer;
    typedef T&  reference;
    static inline void acquire(pointer t) { if (t) { t->ref(); } }
    static inline void release(pointer t) { if (t) { t->unref(); } }
};

}  // namespace sfz

#endif  // SFZ_SMART_PTR_HPP_
