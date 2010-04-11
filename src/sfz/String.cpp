// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/String.hpp"

#include <string.h>
#include <algorithm>
#include "sfz/Bytes.hpp"
#include "sfz/Encoding.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"

using std::max;
using std::min;

namespace sfz {

namespace {

const size_t kDefaultStringSize = 16;

}  // namespace

// String implementation.

String::String() {
    initialize(0);
}

String::String(const String& string) {
    initialize(string._capacity);
    append(StringPiece(string));
}

String::String(const StringPiece& string) {
    initialize(string._size);
    append(string);
}

void String::append(const String& string) {
    append(StringPiece(string));
}

void String::append(const StringPiece& string) {
    reserve(_size + string._size);
    memcpy(_data.get() + _size, string._data, string._size * sizeof(uint32_t));
    _size += string._size;
}

void String::assign(const String& string) {
    clear();
    append(StringPiece(string));
}

void String::assign(const StringPiece& string) {
    clear();
    append(string);
}

String::String(const char* data, const Encoding& encoding) {
    initialize(0);
    append(BytesPiece(reinterpret_cast<const uint8_t*>(data), strlen(data)), encoding);
}

void String::append(const char* data, const Encoding& encoding) {
    append(BytesPiece(reinterpret_cast<const uint8_t*>(data), strlen(data)), encoding);
}

void String::assign(const char* data, const Encoding& encoding) {
    clear();
    append(BytesPiece(reinterpret_cast<const uint8_t*>(data), strlen(data)), encoding);
}

String::String(const BytesPiece& bytes, const Encoding& encoding) {
    initialize(0);
    append(bytes, encoding);
}

void String::append(const BytesPiece& bytes, const Encoding& encoding) {
    encoding.decode(bytes, this);
}

void String::assign(const BytesPiece& bytes, const Encoding& encoding) {
    clear();
    append(bytes, encoding);
}

String::String(size_t num, uint32_t code) {
    initialize(num);
    append(num, code);
}

void String::append(size_t num, uint32_t code) {
    if (!is_valid_code_point(code)) {
        abort();
    }
    resize(_size + num, code);
}

void String::assign(size_t num, uint32_t code) {
    clear();
    append(num, code);
}

String::~String() { }

size_t String::size() const {
    return _size;
}

uint32_t String::at(size_t loc) const {
    if (loc >= _size) {
        abort();
    }
    return _data.get()[loc];
}

void String::clear() {
    _size = 0;
}

bool String::empty() const {
    return _size == 0;
}

void String::reserve(size_t capacity) {
    if (_capacity < capacity) {
        size_t new_capacity = _capacity * 2;
        while (new_capacity < capacity) {
            new_capacity *= 2;
        }
        scoped_array<uint32_t> new_data(new uint32_t[new_capacity]);
        memcpy(new_data.get(), _data.get(), _size * sizeof(uint32_t));
        _data.swap(&new_data);
        _capacity = new_capacity;
    }
}

void String::resize(size_t size, uint32_t code) {
    if (size > _size) {
        reserve(size);
        foreach (i, range(_size, size)) {
            _data.get()[i] = code;
        }
    }
    _size = size;
}

void String::swap(String* string) {
    _data.swap(&string->_data);
    std::swap(_size, string->_size);
    std::swap(_capacity, string->_capacity);
}

size_t String::find(uint32_t code, size_t index) const {
    foreach (i, range(index, size())) {
        if (at(i) == code) {
            return i;
        }
    }
    return kNone;
}

size_t String::find(const StringPiece& string, size_t index) const {
    if (index + string.size() > size()) {
        return kNone;
    }
    foreach (i, range(index, size() - string.size() + 1)) {
        if (StringPiece(*this).substr(i, string.size()) == string) {
            return i;
        }
    }
    return kNone;
}

size_t String::rfind(uint32_t code, size_t index) const {
    if (index == kNone) {
        index = size() - 1;
    }
    foreach (i, range(index + 1)) {
        if (at(index - i) == code) {
            return index - i;
        }
    }
    return kNone;
}

void String::replace(size_t index, size_t num, const StringPiece& string) {
    String tail(StringPiece(*this).substr(index + num));
    resize(index);
    append(string);
    append(tail);
}

/*
StringIterator String::begin() const {
    return StringPiece(*this).begin();
}

StringIterator String::end() const {
    return StringPiece(*this).end();
}
*/

void String::initialize(size_t capacity) {
    capacity = max(capacity, kDefaultStringSize);
    _data.reset(new uint32_t[capacity]);
    _size = 0;
    _capacity = capacity;
}

// StringKey implementation.

bool operator<(const StringKey& lhs, const StringKey& rhs) {
    foreach (i, range(min(lhs.size(), rhs.size()))) {
        if (lhs.at(i) < rhs.at(i)) {
            return true;
        } else if (lhs.at(i) > rhs.at(i)) {
            return false;
        }
    }
    return lhs.size() < rhs.size();
}

// StringPiece implementation.

StringPiece::StringPiece()
    : _data(NULL),
      _size(0) { }

StringPiece::StringPiece(const String& string)
    : _data(string._data.get()),
      _size(string._size) { }

size_t StringPiece::size() const {
    return _size;
}

uint32_t StringPiece::at(size_t loc) const {
    if (loc >= _size) {
        abort();
    }
    return _data[loc];
}

bool StringPiece::empty() const {
    return _size == 0;
}

size_t StringPiece::find(uint32_t code, size_t index) const {
    foreach (i, range(index, size())) {
        if (at(i) == code) {
            return i;
        }
    }
    return kNone;
}

size_t StringPiece::rfind(uint32_t code, size_t index) const {
    if (index == kNone) {
        index = size() - 1;
    }
    foreach (i, range(index + 1)) {
        if (at(index - i) == code) {
            return index - i;
        }
    }
    return kNone;
}

StringPiece StringPiece::substr(size_t loc) const {
    if (loc > _size) {
        abort();
    }
    return StringPiece(_data + loc, _size - loc);
}

StringPiece StringPiece::substr(size_t loc, size_t size) const {
    if (loc + size > _size) {
        abort();
    }
    return StringPiece(_data + loc, size);
}

StringPiece::const_iterator StringPiece::begin() const {
    return const_iterator(_data);
}

StringPiece::const_iterator StringPiece::end() const {
    return const_iterator(_data + _size);
}

StringPiece::StringPiece(const uint32_t* data, size_t size)
    : _data(data),
      _size(size) { }

// StringPiece::const_iterator implementation.

StringPiece::const_iterator::const_iterator(const uint32_t* it)
    : _it(it) { }

StringPiece::const_iterator::value_type StringPiece::const_iterator::operator*() const {
    return *_it;
}

StringPiece::const_iterator& StringPiece::const_iterator::operator++() {
    ++_it;
    return *this;
}

StringPiece::const_iterator StringPiece::const_iterator::operator++(int) {
    const uint32_t* const old = _it;
    ++_it;
    return const_iterator(old);
}

bool StringPiece::const_iterator::operator==(const StringPiece::const_iterator& it) {
    return _it == it._it;
}

bool StringPiece::const_iterator::operator!=(const StringPiece::const_iterator& it) {
    return _it != it._it;
}

bool operator==(const StringPiece& lhs, const StringPiece& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (StringPiece::const_iterator it = lhs.begin(), jt = rhs.begin();
            it != lhs.end() && jt != rhs.end(); ++it, ++jt) {
        if (*it != *jt) {
            return false;
        }
    }
    return true;
}

bool operator!=(const StringPiece& lhs, const StringPiece& rhs) {
    return !(lhs == rhs);
}

}  // namespace sfz
