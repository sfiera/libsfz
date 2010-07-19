// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_LINKED_PTR_HPP_
#define SFZ_LINKED_PTR_HPP_

#include <stdlib.h>
#include <algorithm>
#include <sfz/SmartPtr.hpp>

namespace sfz {

class linked_group {
  public:
    linked_group();

    void join(linked_group* group);
    bool depart();

    bool unique() const;
    void check_unique() const;

  private:
    linked_group* _prev;
    linked_group* _next;

    DISALLOW_COPY_AND_ASSIGN(linked_group);
};

template <typename T, template <typename T> class traits>
class linked {
  public:
    typedef typename traits<T>::value_type  value_type;
    typedef typename traits<T>::pointer     pointer;

    linked(pointer ptr) : _ptr(ptr) { }

    linked(const linked& other) {
        copy(other);
    }

    linked& operator=(const linked& other) {
        if (&other != this) {
            reset();
            copy(other);
        }
        return *this;
    }

    template <typename U>
    linked(const linked<U, traits>& other) {
        copy(other);
    }

    template <typename U>
    linked& operator=(const linked<U, traits>& other) {
        reset();
        copy(other);
        return *this;
    }

    ~linked() { reset(); }

    pointer get() const { return _ptr; }
    bool unique() const { return _group.unique(); }

    pointer release() {
        _group.check_unique();
        pointer ptr = NULL;
        std::swap(ptr, _ptr);
        return ptr;
    }

    void reset(pointer new_ptr = NULL) {
        if (_group.depart() && _ptr) {
            traits<T>::destroy(_ptr);
        }
        _ptr = new_ptr;
    }

    void swap(linked* s) {
        _group.check_unique();
        s->_group.check_unique();
        std::swap(_ptr, s->_ptr);
    }

  private:
    template <typename U, template <typename V> class other_traits>
    friend class linked;

    template <typename U>
    void copy(const linked<U, traits>& other) {
        _ptr = other._ptr;
        _group.join(&other._group);
    }

    pointer _ptr;
    mutable linked_group _group;
};

template <typename T>
class linked_ptr : public linked<T, ptr_traits> {
  public:
    typedef linked<T, ptr_traits>               super;
    typedef typename ptr_traits<T>::pointer     pointer;
    typedef typename ptr_traits<T>::reference   reference;

    explicit linked_ptr(pointer ptr = NULL) : super(ptr) { }
    pointer operator->() const { return this->get(); }
    reference operator*() const { return *this->get(); }

    template <typename U> linked_ptr(const linked_ptr<U>& other) : super(other) { }
    template <typename U> linked_ptr& operator=(const linked_ptr<U>& other) {
        super::operator=(other);
        return *this;
    }
};

template <typename T>
class linked_array : public linked<T, array_traits> {
  public:
    typedef linked<T, array_traits>             super;
    typedef typename array_traits<T>::pointer   pointer;
    typedef typename array_traits<T>::reference reference;

    explicit linked_array(pointer ptr = NULL) : super(ptr) { }
    reference operator[](int index) const { return this->get()[index]; }

    template <typename U> linked_array(const linked_array<U>& other) : super(other) { }
    template <typename U> linked_array& operator=(const linked_array<U>& other) {
        super::operator=(other);
        return *this;
    }
};

template <typename T>
class linked_key : public linked<const T, ptr_traits> {
  public:
    typedef linked<const T, ptr_traits>                 super;
    typedef typename ptr_traits<const T>::value_type    value_type;
    typedef typename ptr_traits<const T>::pointer       pointer;
    typedef typename ptr_traits<const T>::reference     reference;

    explicit linked_key(pointer ptr = NULL) : super(ptr) { }
    pointer operator->() const { return this->get(); }
    reference operator*() const { return *this->get(); }
};
#define SFZ_LINKED_KEY_OP(OP) \
    template <typename T> \
    bool operator OP(const linked_key<T>& lhs, const linked_key<T>& rhs) { \
        return *lhs OP *rhs; \
    }
SFZ_LINKED_KEY_OP(==);
SFZ_LINKED_KEY_OP(!=);
SFZ_LINKED_KEY_OP(<);
SFZ_LINKED_KEY_OP(<=);
SFZ_LINKED_KEY_OP(>);
SFZ_LINKED_KEY_OP(>=);
#undef SFZ_LINKED_KEY_OP

template <typename T>
linked_ptr<T> make_linked_ptr(T* ptr) {
    return linked_ptr<T>(ptr);
}

template <typename T>
linked_key<T> make_linked_key(T* ptr) {
    return linked_key<T>(ptr);
}

}  // namespace sfz

#endif  // SFZ_LINKED_PTR_HPP_
