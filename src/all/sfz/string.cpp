// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/string.hpp"

#include <string.h>
#include <algorithm>
#include "sfz/Exception.hpp"
#include "sfz/algorithm.hpp"
#include "sfz/encoding.hpp"
#include "sfz/foreach.hpp"
#include "sfz/format.hpp"
#include "sfz/macros.hpp"

namespace sfz {

using std::max;
using std::min;
using std::swap;

namespace {

const size_t kDefaultStringSize = 16;

}  // namespace

const size_t String::npos;
const size_t StringSlice::npos;

// String implementation.

String::String() {
    initialize(0);
}

String::String(const String& string) {
    initialize(string._capacity);
    append(PrintItem(string));
}

String::String(const PrintItem& item) {
    initialize(0);
    append(item);
}

void String::append(const PrintItem& item) {
    item.print_to(this);
}

void String::assign(const PrintItem& item) {
    clear();
    append(item);
}

String::String(size_t num, Rune rune) {
    initialize(num);
    append(num, rune);
}

void String::append(size_t num, Rune rune) {
    if (!is_valid_code_point(rune)) {
        throw Exception(format("invalid code point {0}", rune));
    }
    resize(_size + num, rune);
}

void String::assign(size_t num, Rune rune) {
    clear();
    append(num, rune);
}

void String::push(const StringSlice& string) {
    reserve(_size + string._size);
    foreach (Rune r, string) {
        append(1, r);
    }
}

void String::push(size_t num, Rune rune) {
    if (!is_valid_code_point(rune)) {
        throw Exception(format("invalid code point {0}", rune));
    }
    resize(_size + num, rune);
}

String::~String() { }

void String::clear() {
    _size = 0;
}

void String::reserve(size_t capacity) {
    if (_capacity < capacity) {
        size_t new_capacity = _capacity * 2;
        while (new_capacity < capacity) {
            new_capacity *= 2;
        }
        scoped_array<Rune> new_data(new Rune[new_capacity]);
        memcpy(new_data.get(), _data.get(), _size * sizeof(Rune));
        swap(_data, new_data);
        _capacity = new_capacity;
    }
}

void String::resize(size_t size, Rune rune) {
    if (size > _size) {
        reserve(size);
        foreach (size_t i, range(_size, size)) {
            _data[i] = rune;
        }
    }
    _size = size;
}

void String::replace(size_t index, size_t num, const StringSlice& string) {
    String tail(slice(index + num));
    resize(index);
    append(string);
    append(tail);
}

size_t String::size() const {
    return slice().size();
}

Rune String::at(size_t loc) const {
    return slice().at(loc);
}

bool String::empty() const {
    return slice().empty();
}

size_t String::find(Rune rune, size_t index) const {
    return slice().find(rune, index);
}

size_t String::find(const StringSlice& string, size_t index) const {
    return slice().find(string, index);
}

size_t String::rfind(Rune rune, size_t index) const {
    return slice().rfind(rune, index);
}

size_t String::rfind(const StringSlice& string, size_t index) const {
    return slice().rfind(string, index);
}

StringSlice String::slice() const {
    return *this;
}

StringSlice String::slice(size_t loc) const {
    return slice().slice(loc);
}

StringSlice String::slice(size_t loc, size_t size) const {
    return slice().slice(loc, size);
}

void String::initialize(size_t capacity) {
    capacity = max(capacity, kDefaultStringSize);
    _data.reset(new Rune[capacity]);
    _size = 0;
    _capacity = capacity;
}

// StringSlice implementation.

StringSlice::StringSlice()
        : _data(NULL),
          _encoding(sizeof(uint8_t)),
          _size(0) { }

StringSlice::StringSlice(const String& string)
        : _data(reinterpret_cast<uint8_t*>(string._data.get())),
          _encoding(sizeof(Rune)),
          _size(string._size) { }

StringSlice::StringSlice(const char* ascii_string) {
    BytesSlice bytes(reinterpret_cast<const uint8_t*>(ascii_string), strlen(ascii_string));
    foreach (uint8_t byte, bytes) {
        if (byte & 0x80) {
            throw Exception("string is not ASCII");
        }
    }
    _data = bytes.data();
    _encoding = sizeof(uint8_t);
    _size = bytes.size();
}

size_t StringSlice::size() const {
    return _size;
}

Rune StringSlice::at(size_t loc) const {
    if (loc >= _size) {
        throw Exception("out-of-bounds");
    }
    switch (_encoding) {
      case sizeof(uint8_t):
        return _data[loc];
      case sizeof(uint16_t):
        return reinterpret_cast<const uint16_t*>(_data)[loc];
      case sizeof(uint32_t):
        return reinterpret_cast<const uint32_t*>(_data)[loc];
    }
    return '\0';
}

bool StringSlice::empty() const {
    return _size == 0;
}

size_t StringSlice::find(Rune rune, size_t index) const {
    foreach (size_t i, range(index, _size)) {
        if (at(i) == rune) {
            return i;
        }
    }
    return npos;
}

size_t StringSlice::find(const StringSlice& string, size_t index) const {
    if (index + string._size > _size) {
        return npos;
    }
    foreach (size_t i, range(index, _size - string._size + 1)) {
        if (slice(i, string._size) == string) {
            return i;
        }
    }
    return npos;
}

size_t StringSlice::rfind(Rune rune, size_t index) const {
    if (index == npos) {
        index = _size - 1;
    }
    foreach (size_t i, range(index + 1)) {
        if (at(index - i) == rune) {
            return index - i;
        }
    }
    return npos;
}

size_t StringSlice::rfind(const StringSlice& string, size_t index) const {
    if (string._size > _size) {
        return npos;
    }
    if (index == npos) {
        index = _size;
    }
    if (index + string._size > _size) {
        index = _size - string._size;
    }
    foreach (size_t i, range(index - string._size + 1)) {
        if (slice(index - i, string._size) == string) {
            return index - i;
        }
    }
    return npos;
}

StringSlice StringSlice::slice() const {
    return *this;
}

StringSlice StringSlice::slice(size_t loc) const {
    if (loc > _size) {
        throw Exception("slice out of range");
    }
    return slice(loc, _size - loc);
}

StringSlice StringSlice::slice(size_t loc, size_t size) const {
    if (loc + size > _size) {
        throw Exception("slice out of range");
    }
    return StringSlice(_data + (loc * _encoding), size, _encoding);
}

StringSlice::const_iterator StringSlice::begin() const {
    return const_iterator(_data, _encoding);
}

StringSlice::const_iterator StringSlice::end() const {
    return const_iterator(_data + (_size * _encoding), _encoding);
}

StringSlice::StringSlice(const uint8_t* data, size_t size, int encoding)
        : _data(data),
          _encoding(encoding),
          _size(size) { }

// StringSlice::const_iterator implementation.

StringSlice::iterator::iterator() { }

StringSlice::iterator::iterator(const uint8_t* it, int encoding)
        : _it(it),
          _encoding(encoding) { }

StringSlice::iterator::value_type StringSlice::iterator::operator*() const {
    switch (_encoding) {
      case sizeof(uint8_t):
        return *_it;
      case sizeof(uint16_t):
        return *reinterpret_cast<const uint16_t*>(_it);
      case sizeof(uint32_t):
        return *reinterpret_cast<const uint32_t*>(_it);
    }
    return 0;
}

StringSlice::iterator& StringSlice::iterator::operator++() {
    _it += _encoding;
    return *this;
}

StringSlice::iterator StringSlice::iterator::operator++(int) {
    const uint8_t* const old = _it;
    _it += _encoding;
    return iterator(old, _encoding);
}

StringSlice::iterator& StringSlice::iterator::operator--() {
    _it -= _encoding;
    return *this;
}

StringSlice::iterator StringSlice::iterator::operator--(int) {
    const uint8_t* const old = _it;
    _it -= _encoding;
    return iterator(old, _encoding);
}

StringSlice::iterator& StringSlice::iterator::operator+=(int n) {
    _it += _encoding * n;
    return *this;
}

StringSlice::iterator StringSlice::iterator::operator+(int n) const {
    return iterator(_it + (_encoding * n), _encoding);
}

StringSlice::iterator& StringSlice::iterator::operator-=(int n) {
    _it -= _encoding * n;
    return *this;
}

StringSlice::iterator StringSlice::iterator::operator-(int n) const {
    return iterator(_it - (_encoding * n), _encoding);
}

StringSlice::difference_type StringSlice::iterator::operator-(const iterator& it) const {
    return (_it - it._it) / _encoding;
}

void swap(String& x, String& y) {
    swap(x._data, y._data);
    swap(x._size, y._size);
    swap(x._capacity, y._capacity);
}

void swap(StringSlice& x, StringSlice& y) {
    swap(x._data, y._data);
    swap(x._size, y._size);
}

int compare(const String& x, const String& y) {
    return compare(StringSlice(x), StringSlice(y));
}

int compare(const StringSlice& x, const StringSlice& y) {
    using sfz::compare;

    for (StringSlice::iterator xit = x.begin(), yit = y.begin(), xend = x.end(), yend = y.end();
            true; ++xit, ++yit) {
        if ((xit == xend) && (yit == yend)) {
            return 0;
        } else if (xit == xend) {
            return -1;
        } else if (yit == yend) {
            return 1;
        } else if (compare(*xit, *yit) != 0) {
            return compare(*xit, *yit);
        }
    }
}

int compare(const StringSlice::iterator& x, const StringSlice::iterator& y) {
    using sfz::compare;

    return sfz::compare(x._it, y._it);
}

SFZ_OPERATORS_BASED_ON_COMPARE(String)
SFZ_OPERATORS_BASED_ON_COMPARE(StringSlice)
SFZ_OPERATORS_BASED_ON_COMPARE(StringSlice::iterator)

}  // namespace sfz
