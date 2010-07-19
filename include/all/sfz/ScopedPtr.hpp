// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_SCOPED_PTR_HPP_
#define SFZ_SCOPED_PTR_HPP_

#include <stdlib.h>
#include <algorithm>
#include <sfz/Macros.hpp>
#include <sfz/SmartPtr.hpp>

namespace sfz {

template <typename T, template <typename T> class traits>
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
        pointer ptr = NULL;
        std::swap(ptr, _ptr);
        return ptr;
    }

    void reset(pointer new_ptr = NULL) {
        traits<T>::release(_ptr);
        _ptr = new_ptr;
        traits<T>::acquire(_ptr);
    }

    void swap(scoped* s) {
        std::swap(_ptr, s->_ptr);
    }

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

template <typename T>
class scoped_ref : public scoped<T, ref_traits> {
  public:
    typedef scoped<T, ref_traits>               super;
    typedef typename ref_traits<T>::pointer     pointer;
    typedef typename ref_traits<T>::reference   reference;

    explicit scoped_ref(pointer ptr = NULL) : super(ptr) { }
    pointer operator->() const { return this->get(); }
    reference operator*() const { return *this->get(); }
};

}  // namespace sfz

#endif  // SFZ_SCOPED_PTR_HPP_
