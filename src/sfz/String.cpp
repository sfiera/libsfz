// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/String.hpp"

#include <string.h>
#include <algorithm>
#include "sfz/Bytes.hpp"
#include "sfz/Encoding.hpp"
#include "sfz/Exception.hpp"
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
    foreach (it, string) {
        append(1, *it);
    }
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

String::String(size_t num, Rune rune) {
    initialize(num);
    append(num, rune);
}

void String::append(size_t num, Rune rune) {
    if (!is_valid_code_point(rune)) {
        throw Exception("invalid code point {0}", rune);
    }
    resize(_size + num, rune);
}

void String::assign(size_t num, Rune rune) {
    clear();
    append(num, rune);
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
        _data.swap(&new_data);
        _capacity = new_capacity;
    }
}

void String::resize(size_t size, Rune rune) {
    if (size > _size) {
        reserve(size);
        foreach (i, range(_size, size)) {
            _data.get()[i] = rune;
        }
    }
    _size = size;
}

void String::swap(String* string) {
    _data.swap(&string->_data);
    std::swap(_size, string->_size);
    std::swap(_capacity, string->_capacity);
}

void String::replace(size_t index, size_t num, const StringPiece& string) {
    String tail(StringPiece(*this).substr(index + num));
    resize(index);
    append(string);
    append(tail);
}

size_t String::size() const {
    return StringPiece(*this).size();
}

Rune String::at(size_t loc) const {
    return StringPiece(*this).at(loc);
}

bool String::empty() const {
    return StringPiece(*this).empty();
}

size_t String::find(Rune rune, size_t index) const {
    return StringPiece(*this).find(rune, index);
}

size_t String::find(const StringPiece& string, size_t index) const {
    return StringPiece(*this).find(string, index);
}

size_t String::rfind(Rune rune, size_t index) const {
    return StringPiece(*this).rfind(rune, index);
}

size_t String::rfind(const StringPiece& string, size_t index) const {
    return StringPiece(*this).rfind(string, index);
}

StringPiece String::substr(size_t loc) const {
    return StringPiece(*this).substr(loc);
}

StringPiece String::substr(size_t loc, size_t size) const {
    return StringPiece(*this).substr(loc, size);
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
    _data.reset(new Rune[capacity]);
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
          _encoding(sizeof(uint8_t)),
          _size(0) { }

StringPiece::StringPiece(const String& string)
        : _data(reinterpret_cast<uint8_t*>(string._data.get())),
          _encoding(sizeof(Rune)),
          _size(string._size) { }

StringPiece::StringPiece(const char* ascii_string) {
    BytesPiece bytes(reinterpret_cast<const uint8_t*>(ascii_string), strlen(ascii_string));
    if (!ascii_encoding().can_decode(bytes)) {
        throw Exception("string is not ASCII");
    }
    _data = bytes.data();
    _encoding = sizeof(uint8_t);
    _size = bytes.size();
}

size_t StringPiece::size() const {
    return _size;
}

Rune StringPiece::at(size_t loc) const {
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

bool StringPiece::empty() const {
    return _size == 0;
}

size_t StringPiece::find(Rune rune, size_t index) const {
    foreach (i, range(index, _size)) {
        if (at(i) == rune) {
            return i;
        }
    }
    return kNone;
}

size_t StringPiece::find(const StringPiece& string, size_t index) const {
    if (index + string._size > _size) {
        return kNone;
    }
    foreach (i, range(index, _size - string._size + 1)) {
        if (StringPiece(*this).substr(i, string._size) == string) {
            return i;
        }
    }
    return kNone;
}

size_t StringPiece::rfind(Rune rune, size_t index) const {
    if (index == kNone) {
        index = _size - 1;
    }
    foreach (i, range(index + 1)) {
        if (at(index - i) == rune) {
            return index - i;
        }
    }
    return kNone;
}

size_t StringPiece::rfind(const StringPiece& string, size_t index) const {
    if (string._size > _size) {
        return kNone;
    }
    if (index == kNone) {
        index = _size;
    }
    if (index + string._size > _size) {
        index = _size - string._size;
    }
    foreach (i, range(index - string._size + 1)) {
        if (StringPiece(*this).substr(index - i, string._size) == string) {
            return index - i;
        }
    }
    return kNone;
}

StringPiece StringPiece::substr(size_t loc) const {
    if (loc > _size) {
        throw Exception("substr out of range");
    }
    return substr(loc, _size - loc);
}

StringPiece StringPiece::substr(size_t loc, size_t size) const {
    if (loc + size > _size) {
        throw Exception("substr out of range");
    }
    return StringPiece(_data + (loc * _encoding), size, _encoding);
}

StringPiece::const_iterator StringPiece::begin() const {
    return const_iterator(_data, _encoding);
}

StringPiece::const_iterator StringPiece::end() const {
    return const_iterator(_data + (_size * _encoding), _encoding);
}

StringPiece::StringPiece(const uint8_t* data, size_t size, int encoding)
        : _data(data),
          _encoding(encoding),
          _size(size) { }

// StringPiece::const_iterator implementation.

StringPiece::const_iterator::const_iterator(const uint8_t* it, int encoding)
        : _it(it),
          _encoding(encoding) { }

StringPiece::const_iterator::value_type StringPiece::const_iterator::operator*() const {
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

StringPiece::const_iterator& StringPiece::const_iterator::operator++() {
    _it += _encoding;
    return *this;
}

StringPiece::const_iterator StringPiece::const_iterator::operator++(int) {
    const uint8_t* const old = _it;
    _it += _encoding;
    return const_iterator(old, _encoding);
}

bool StringPiece::const_iterator::operator==(const StringPiece::const_iterator& it) {
    return _it == it._it;
}

bool StringPiece::const_iterator::operator!=(const StringPiece::const_iterator& it) {
    return _it != it._it;
}

bool operator==(const String& lhs, const String& rhs) {
    return StringPiece(lhs) == StringPiece(rhs);
}

bool operator!=(const String& lhs, const String& rhs) {
    return StringPiece(lhs) != StringPiece(rhs);
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
