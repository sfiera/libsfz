// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_OPTIONAL_HPP_
#define SFZ_OPTIONAL_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <sfz/algorithm.hpp>
#include <stdexcept>

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

    template <typename... Args>
    void set(Args&&... args);

    T* get() { return null_or(data()); }
    T* operator->() { return exception_or(data()); }
    T& operator*() { return *exception_or(data()); }

    const T* get() const { return null_or(data()); }
    const T* operator->() const { return exception_or(data()); }
    const T& operator*() const { return *exception_or(data()); }

  private:
    T*       data();
    const T* data() const;
    template <typename U>
    U null_or(U u) const;
    template <typename U>
    U exception_or(U u) const;

    bool    _has;
    uint8_t _data[sizeof(T)];
};

template <typename T>
void copy(Optional<T>& to, const T& from);
template <typename T>
void copy(Optional<T>& to, const Optional<T>& from);

template <typename T>
Optional<T>::Optional() : _has(false) {}

template <typename T>
Optional<T>::Optional(const Optional& other) : _has(false) {
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
template <typename... Args>
void Optional<T>::set(Args&&... args) {
    clear();
    new (data()) T(std::forward<Args&&>(args)...);
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
    throw std::runtime_error("!has()");
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

}  // namespace sfz

#endif  // SFZ_OPTIONAL_HPP_
