// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_MEMORY_HPP_
#define SFZ_MEMORY_HPP_

#include <stdlib.h>
#include <algorithm>
#include <sfz/macros.hpp>

namespace sfz {

template <typename T, template <typename> class traits> class scoped;
template <typename T, template <typename> class traits>
inline void swap(scoped<T, traits>& x, scoped<T, traits>& y);

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

template <typename T, template <typename> class traits>
class scoped {
  public:
    typedef typename traits<T>::value_type  value_type;
    typedef typename traits<T>::pointer     pointer;

    scoped(pointer ptr)
            : _ptr(ptr) {
        traits<T>::acquire(_ptr);
    }

    scoped(const scoped& other)
            : _ptr(other._ptr) {
        traits<T>::acquire(_ptr);
    }

    scoped& operator=(const scoped& other) {
        if (&other != this) {
            reset(other._ptr);
        }
        return *this;
    }

    ~scoped() { reset(); }

    pointer get() const { return _ptr; }

    pointer release() {
        using std::swap;
        pointer ptr = NULL;
        swap(ptr, _ptr);
        return ptr;
    }

    void reset(pointer new_ptr = NULL) {
        traits<T>::release(_ptr);
        _ptr = new_ptr;
        traits<T>::acquire(_ptr);
    }

    friend void swap<>(scoped& x, scoped& y);

  private:
    pointer _ptr;
};

template <typename T>
class scoped_ptr : public scoped<T, ptr_traits> {
  public:
    typedef scoped<T, ptr_traits>               super;
    typedef typename ptr_traits<T>::pointer     pointer;
    typedef typename ptr_traits<T>::reference   reference;

    explicit scoped_ptr(pointer ptr = NULL) : super(ptr) { }
    pointer operator->() const { return this->get(); }
    reference operator*() const { return *this->get(); }

    DISALLOW_COPY_AND_ASSIGN(scoped_ptr);
};

template <typename T>
class scoped_array : public scoped<T, array_traits> {
  public:
    typedef scoped<T, array_traits>             super;
    typedef typename array_traits<T>::pointer   pointer;
    typedef typename array_traits<T>::reference reference;

    explicit scoped_array(pointer ptr = NULL) : super(ptr) { }
    reference operator[](int index) const { return this->get()[index]; }

    DISALLOW_COPY_AND_ASSIGN(scoped_array);
};

template <typename T, template <typename> class traits>
inline void swap(scoped<T, traits>& x, scoped<T, traits>& y) {
    using std::swap;
    swap(x._ptr, y._ptr);
}

template <typename T>
inline void swap(scoped_ptr<T>& x, scoped_ptr<T>& y) {
    typename scoped_ptr<T>::super& super_x = x;
    typename scoped_ptr<T>::super& super_y = y;
    swap(super_x, super_y);
}

template <typename T>
inline void swap(scoped_array<T>& x, scoped_array<T>& y) {
    typename scoped_array<T>::super& super_x = x;
    typename scoped_array<T>::super& super_y = y;
    swap(super_x, super_y);
}

}  // namespace sfz

#endif  // SFZ_MEMORY_HPP_
