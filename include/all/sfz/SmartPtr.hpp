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
class scoped_ptr {
  public:
    explicit scoped_ptr(T* ptr = NULL) : _ptr(ptr) { }

    ~scoped_ptr() { reset(); }

    T* get() const { return _ptr; }
    T* operator->() const { return _ptr; }
    T& operator*() const { return *_ptr; }

    T* release() {
        T* ptr = _ptr;
        _ptr = NULL;
        return ptr;
    }

    void reset(T* new_ptr = NULL) {
        if (_ptr) {
            delete _ptr;
        }
        _ptr = new_ptr;
    }

    void swap(scoped_ptr<T>* s) {
        std::swap(_ptr, s->_ptr);
    }

  private:
    T* _ptr;

    DISALLOW_COPY_AND_ASSIGN(scoped_ptr);
};

template <typename T>
class scoped_array {
  public:
    explicit scoped_array(T* ptr = NULL) : _ptr(ptr) { }

    ~scoped_array() { reset(); }

    T* get() const { return _ptr; }
    T* operator->() const { return _ptr; }
    T& operator*() const { return *_ptr; }

    T* release() {
        T* ptr = _ptr;
        _ptr = NULL;
        return ptr;
    }

    void reset(T* new_ptr = NULL) {
        if (_ptr) {
            delete [] _ptr;
        }
        _ptr = new_ptr;
    }

    void swap(scoped_array<T>* s) {
        std::swap(_ptr, s->_ptr);
    }

  private:
    T* _ptr;

    DISALLOW_COPY_AND_ASSIGN(scoped_array);
};

}  // namespace sfz

#endif  // SFZ_SMART_PTR_HPP_
