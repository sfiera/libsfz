// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_OPTIONAL_HPP_
#define SFZ_OPTIONAL_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <stdexcept>

namespace sfz {

template <typename T>
class optional {
  public:
    optional();
    optional(const optional& other);
    optional& operator=(const optional& other);
    ~optional();

    constexpr operator bool() const { return has_value(); }
    constexpr bool has_value() const { return _has_value; }

    T* operator->() { return data(); }
    const T* operator->() const { return data(); }
    T& operator*() { return *data(); }
    const T& operator*() const { return *data(); }

    T&       value() { return *exception_or(data()); }
    const T& value() const { return *exception_or(data()); }

    void swap(optional& other);
    void reset();
    template <typename... Args>
    void emplace(Args&&... args);

  private:
    T*       data();
    const T* data() const;

    template <typename U>
    U exception_or(U u) const;

    bool    _has_value;
    uint8_t _data[sizeof(T)];
};

template <typename T>
optional<T>::optional() : _has_value(false) {}

template <typename T>
optional<T>::optional(const optional& other) : _has_value(false) {
    if (other.has_value()) {
        emplace(*other);
    }
}

template <typename T>
optional<T> make_optional(T&& value) {
    optional<T> o;
    o.emplace(std::forward<T&&>(value));
    return o;
}

template <typename T>
optional<T>& optional<T>::operator=(const optional& other) {
    if (other.has_value()) {
        if (has_value()) {
            **this = *other;
        } else {
            emplace(*other);
        }
    } else {
        reset();
    }
    return *this;
}

template <typename T>
optional<T>::~optional() {
    reset();
}

template <typename T>
void optional<T>::reset() {
    if (_has_value) {
        data()->~T();
        _has_value = false;
    }
}

template <typename T>
template <typename... Args>
void optional<T>::emplace(Args&&... args) {
    reset();
    new (data()) T(std::forward<Args&&>(args)...);
    _has_value = true;
}

template <typename T>
T* optional<T>::data() {
    return reinterpret_cast<T*>(_data);
}

template <typename T>
const T* optional<T>::data() const {
    return reinterpret_cast<const T*>(_data);
}

template <typename T>
template <typename U>
U optional<T>::exception_or(U u) const {
    if (_has_value) {
        return u;
    }
    throw std::runtime_error("!has_value()");
}

}  // namespace sfz

#endif  // SFZ_OPTIONAL_HPP_
