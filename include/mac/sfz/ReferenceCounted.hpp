// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_REFERENCE_COUNTED_HPP_
#define SFZ_REFERENCE_COUNTED_HPP_

#include <stdint.h>
#include <algorithm>
#include <sfz/Macros.hpp>

namespace sfz {

class ReferenceCounted {
  public:
    ReferenceCounted();
    virtual ~ReferenceCounted();

    void ref() const;
    bool unref() const;

  private:
    mutable int32_t _reference_count;

    DISALLOW_COPY_AND_ASSIGN(ReferenceCounted);
};

template <typename T>
class RefPtr {
  public:
    RefPtr(T* ptr = NULL) : _ptr(NULL) { reset(ptr); }

    RefPtr(const RefPtr& p) : _ptr(NULL) { reset(p.get()); }
    RefPtr& operator=(const RefPtr& p) { reset(p.get()); return *this; }

    ~RefPtr() { reset(); }

    T* get() const { return _ptr; }
    T* operator->() const { return _ptr; }
    T& operator*() const { return *_ptr; }

    T* release() {
        T* ptr = _ptr;
        _ptr = NULL;
        return ptr;
    }

    void reset(T* ptr = NULL) {
        if (ptr) {
            ptr->ref();
        }
        if (_ptr) {
            _ptr->unref();
        }
        _ptr = ptr;
    }

    void swap(RefPtr* p) {
        std::swap(_ptr, p->_ptr);
    }

  private:
    T* _ptr;

    // ALLOW_COPY_AND_ASSIGN
};

}  // namespace sfz

#endif  // SFZ_REFERENCE_COUNTED_HPP_
