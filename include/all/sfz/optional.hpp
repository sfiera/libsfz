// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_OPTIONAL_HPP_
#define SFZ_OPTIONAL_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <sfz/algorithm.hpp>
#include <sfz/exception.hpp>

namespace sfz {

template <typename T>
class Optional {
  public:
    Optional();
    Optional(const Optional& other);
    Optional& operator=(const Optional& other);
    ~Optional();

    bool has() const { return _has; }
    void clear();

    void set();
    template <typename A0>
    void set(const A0& a0);
    template <typename A0, typename A1>
    void set(const A0& a0, const A1& a1);

    T* get() { return null_or(data()); }
    T* operator->() { return exception_or(data()); }
    T& operator*() { return *exception_or(data()); }

    const T* get() const { return null_or(data()); }
    const T* operator->() const { return exception_or(data()); }
    const T& operator*() const { return *exception_or(data()); }

  private:
    T* data();
    const T* data() const;
    template <typename U> U null_or(U u) const;
    template <typename U> U exception_or(U u) const;

    bool _has;
    uint8_t _data[sizeof(T)];
};

template <typename T> void copy(Optional<T>& to, const T& from);
template <typename T> void copy(Optional<T>& to, const Optional<T>& from);

template <typename T> bool store_argument(Optional<T>& to, StringSlice from);

template <typename T>
Optional<T>::Optional():
        _has(false) { }

template <typename T>
Optional<T>::Optional(const Optional& other):
        _has(false) {
    if (other.has()) {
        set(*other);
    }
}

template <typename T>
Optional<T>& Optional<T>::operator=(const Optional& other) {
    if (other.has()) {
        if (has()) {
            **this = *other;
        } else {
            set(*other);
        }
    } else {
        clear();
    }
    return *this;
}

template <typename T>
Optional<T>::~Optional() {
    clear();
}

template <typename T>
void Optional<T>::clear() {
    if (_has) {
        data()->~T();
        _has = false;
    }
}

template <typename T>
void Optional<T>::set() {
    clear();
    new (data()) T();
    _has = true;
}

template <typename T>
template <typename A0>
void Optional<T>::set(const A0& a0) {
    clear();
    new (data()) T(a0);
    _has = true;
}

template <typename T>
template <typename A0, typename A1>
void Optional<T>::set(const A0& a0, const A1& a1) {
    clear();
    new (data()) T(a0, a1);
    _has = true;
}

template <typename T>
T* Optional<T>::data() {
    return reinterpret_cast<T*>(_data);
}

template <typename T>
const T* Optional<T>::data() const {
    return reinterpret_cast<const T*>(_data);
}

template <typename T>
template <typename U>
U Optional<T>::null_or(U u) const {
    if (_has) {
        return u;
    }
    return NULL;
}

template <typename T>
template <typename U>
U Optional<T>::exception_or(U u) const {
    if (_has) {
        return u;
    }
    throw Exception("!has()");
}

template <typename T>
void copy(Optional<T>& to, const T& from) {
    if (to.has()) {
        copy(*to, from);
    } else {
        to.set(from);
    }
}

template <typename T>
void copy(Optional<T>& to, const Optional<T>& from) {
    if (from.has()) {
        copy(to, *from);
    } else {
        to.clear();
    }
}

template <typename T>
bool store_argument(Optional<T>& to, StringSlice from, PrintTarget error) {
    using std::swap;
    to.set();
    T value;
    if (!store_argument(value, from, error)) {
        return false;
    }
    swap(*to, value);
    return true;
}

}  // namespace sfz

#endif  // SFZ_OPTIONAL_HPP_
