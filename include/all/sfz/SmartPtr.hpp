// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_SMART_PTR_HPP_
#define SFZ_SMART_PTR_HPP_

#include <stdlib.h>
#include <algorithm>
#include <sfz/Macros.hpp>

namespace sfz {

template <typename T>
struct ptr_traits {
    typedef T   value_type;
    typedef T*  pointer;
    typedef T&  reference;
    static inline void destroy(pointer t) { delete t; }
};

template <typename T>
struct array_traits {
    typedef T   value_type;
    typedef T*  pointer;
    typedef T&  reference;
    static inline void destroy(pointer t) { delete[] t; }
};

template <typename T, template <typename T> class traits>
class scoped {
  public:
    typedef typename traits<T>::value_type  value_type;
    typedef typename traits<T>::pointer     pointer;

    scoped(pointer ptr) : _ptr(ptr) { }

    ~scoped() { reset(); }

    pointer get() const { return _ptr; }

    pointer release() {
        pointer ptr = NULL;
        std::swap(ptr, _ptr);
        return ptr;
    }

    void reset(pointer new_ptr = NULL) {
        if (this->_ptr) {
            traits<T>::destroy(this->_ptr);
        }
        this->_ptr = new_ptr;
    }

    void swap(scoped* s) {
        std::swap(_ptr, s->_ptr);
    }

  private:
    pointer _ptr;

    DISALLOW_COPY_AND_ASSIGN(scoped);
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
};

template <typename T>
class scoped_array : public scoped<T, array_traits> {
  public:
    typedef scoped<T, array_traits>             super;
    typedef typename array_traits<T>::pointer   pointer;
    typedef typename array_traits<T>::reference reference;

    explicit scoped_array(pointer ptr = NULL) : super(ptr) { }
    reference operator[](int index) const { return this->get()[index]; }
};

}  // namespace sfz

#endif  // SFZ_SMART_PTR_HPP_
