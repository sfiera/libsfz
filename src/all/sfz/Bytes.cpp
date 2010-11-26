// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/Bytes.hpp"

#include <algorithm>
#include "sfz/Exception.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"

namespace sfz {

using std::max;
using std::min;
using std::swap;

namespace {

const size_t kDefaultBytesSize = 16;

}  // namespace

const size_t Bytes::npos = -1;
const size_t BytesPiece::npos = -1;

Bytes::Bytes()
    : _data(new uint8_t[kDefaultBytesSize]),
      _size(0),
      _capacity(kDefaultBytesSize) { }

Bytes::Bytes(const Bytes& bytes)
    : _data(new uint8_t[bytes._capacity]),
      _size(bytes._size),
      _capacity(bytes._capacity) {
    memcpy(_data.get(), bytes._data.get(), _size);
}

Bytes::Bytes(const BytesPiece& bytes)
    : _data(new uint8_t[max(bytes.size(), kDefaultBytesSize)]),
      _size(bytes.size()),
      _capacity(max(bytes.size(), kDefaultBytesSize)) {
    memcpy(_data.get(), bytes.data(), _size);
}

Bytes::Bytes(const char* data)
    : _data(new uint8_t[max(strlen(data), kDefaultBytesSize)]),
      _size(strlen(data)),
      _capacity(max(strlen(data), kDefaultBytesSize)) {
    memcpy(_data.get(), data, _size);
}

Bytes::Bytes(const uint8_t* data, size_t size)
    : _data(new uint8_t[max(size, kDefaultBytesSize)]),
      _size(size),
      _capacity(max(size, kDefaultBytesSize)) {
    memcpy(_data.get(), data, size);
}

Bytes::Bytes(WriteItem item)
    : _data(new uint8_t[kDefaultBytesSize]),
      _size(0),
      _capacity(kDefaultBytesSize) {
    item.write_to(this);
}

Bytes::Bytes(size_t num, uint8_t byte)
    : _data(new uint8_t[max(num, kDefaultBytesSize)]),
      _size(num),
      _capacity(max(num, kDefaultBytesSize)) {
    memset(_data.get(), byte, num);
}

Bytes::~Bytes() { }

const uint8_t* Bytes::data() const {
    return _data.get();
}

uint8_t* Bytes::mutable_data() const {
    return _data.get();
}

size_t Bytes::size() const {
    return _size;
}

void Bytes::append(const Bytes& bytes) {
    append(bytes.data(), bytes.size());
}

void Bytes::append(const BytesPiece& bytes) {
    append(bytes.data(), bytes.size());
}

void Bytes::append(const char* data) {
    append(reinterpret_cast<const uint8_t*>(data), strlen(data));
}

void Bytes::append(const uint8_t* data, size_t size) {
    reserve(size + _size);
    memcpy(_data.get() + _size, data, size);
    _size += size;
}

void Bytes::append(WriteItem item) {
    item.write_to(this);
}

void Bytes::append(size_t num, uint8_t byte) {
    reserve(num + _size);
    memset(_data.get() + _size, byte, num);
    _size += num;
}

void Bytes::assign(const Bytes& bytes) {
    assign(bytes.data(), bytes.size());
}

void Bytes::assign(const BytesPiece& bytes) {
    assign(bytes.data(), bytes.size());
}

void Bytes::assign(const char* data) {
    assign(reinterpret_cast<const uint8_t*>(data), strlen(data));
}

void Bytes::assign(const uint8_t* data, size_t size) {
    reserve(size);
    memcpy(_data.get(), data, size);
    _size = size;
}

void Bytes::assign(WriteItem item) {
    clear();
    item.write_to(this);
}

void Bytes::assign(size_t num, uint8_t byte) {
    reserve(num);
    memset(_data.get(), byte, num);
    _size = num;
}

uint8_t Bytes::at(size_t loc) const {
    return BytesPiece(*this).at(loc);
}

size_t Bytes::find(uint8_t byte) const {
    return BytesPiece(*this).find(byte);
}

size_t Bytes::find(const BytesPiece& bytes) const {
    return BytesPiece(*this).find(bytes);
}

size_t Bytes::rfind(uint8_t byte) const {
    return BytesPiece(*this).rfind(byte);
}

size_t Bytes::rfind(const BytesPiece& bytes) const {
    return BytesPiece(*this).rfind(bytes);
}

void Bytes::clear() {
    _size = 0;
}

bool Bytes::empty() const {
    return _size == 0;
}

BytesPiece Bytes::substr(size_t index) const {
    return BytesPiece(*this).substr(index);
}

BytesPiece Bytes::substr(size_t index, size_t size) const {
    return BytesPiece(*this).substr(index, size);
}

void Bytes::replace(size_t index, size_t num, const BytesPiece& bytes) {
    Bytes tail(substr(index + num));
    resize(index);
    append(bytes);
    append(tail);
}

void Bytes::reserve(size_t capacity) {
    if (_capacity < capacity) {
        size_t new_capacity = _capacity * 2;
        while (new_capacity < capacity) {
            new_capacity *= 2;
        }
        scoped_array<uint8_t> new_data(new uint8_t[new_capacity]);
        memcpy(new_data.get(), _data.get(), _size);
        swap(_data, new_data);
        _capacity = new_capacity;
    }
}

void Bytes::resize(size_t size, uint8_t byte) {
    if (size < _size) {
        _size = size;
    } else {
        reserve(size);
        memset(_data.get() + _size, byte, size - _size);
        _size = size;
    }
}

BytesPiece::BytesPiece()
    : _data(NULL),
      _size(0) { }

BytesPiece::BytesPiece(const Bytes& bytes)
    : _data(bytes.data()),
      _size(bytes.size()) { }

BytesPiece::BytesPiece(const char* data)
    : _data(reinterpret_cast<const uint8_t*>(data)),
      _size(strlen(data)) { }

BytesPiece::BytesPiece(const uint8_t* data, size_t size)
    : _data(data),
      _size(size) { }

const uint8_t* BytesPiece::data() const {
    return _data;
}

size_t BytesPiece::size() const {
    return _size;
}

uint8_t BytesPiece::at(size_t loc) const {
    if (loc >= _size) {
        throw Exception("out-of-bounds");
    }
    return _data[loc];
}

size_t BytesPiece::find(uint8_t byte) const {
    foreach (i, range(_size)) {
        if (at(i) == byte) {
            return i;
        }
    }
    return BytesPiece::npos;
}

size_t BytesPiece::find(const BytesPiece& bytes) const {
    if (_size < bytes.size()) {
        return BytesPiece::npos;
    }
    foreach (i, range(_size - bytes.size() + 1)) {
        if (substr(i, bytes.size()) == bytes) {
            return i;
        }
    }
    return BytesPiece::npos;
}

size_t BytesPiece::rfind(uint8_t byte) const {
    foreach (i, range(_size)) {
        if (at(_size - i - 1) == byte) {
            return _size - i - 1;
        }
    }
    return BytesPiece::npos;
}

size_t BytesPiece::rfind(const BytesPiece& bytes) const {
    if (_size < bytes.size()) {
        return BytesPiece::npos;
    }
    foreach (i, range(_size - bytes.size() + 1)) {
        if (substr(_size - bytes.size() - i, bytes.size()) == bytes) {
            return _size - bytes.size() - i;
        }
    }
    return BytesPiece::npos;
}

bool BytesPiece::empty() const {
    return _size == 0;
}

BytesPiece BytesPiece::substr(size_t index) const {
    if (index > _size) {
        throw Exception("out-of-bounds");
    }
    return BytesPiece(_data + index, _size - index);
}

BytesPiece BytesPiece::substr(size_t index, size_t size) const {
    if (index + size > _size) {
        throw Exception("out-of-bounds");
    }
    return BytesPiece(_data + index, size);
}

void BytesPiece::shift(size_t size) {
    if (size > _size) {
        throw Exception("out-of-bounds");
    }
    _data += size;
    _size -= size;
}

void BytesPiece::shift(uint8_t* data, size_t size) {
    shift(size);
    memcpy(data, _data - size, size);
}

void swap(Bytes& x, Bytes& y) {
    swap(x._data, y._data);
    swap(x._size, y._size);
    swap(x._capacity, y._capacity);
}

void swap(BytesPiece& x, BytesPiece& y) {
    swap(x._data, y._data);
    swap(x._size, y._size);
}

// Equality operators.

int compare(const Bytes& x, const Bytes& y) {
    return compare(BytesPiece(x), BytesPiece(y));
}

int compare(const BytesPiece& x, const BytesPiece& y) {
    int result = memcmp(x.data(), y.data(), min(x.size(), y.size()));
    if (result) {
        return result;
    }
    if (x.size() < y.size()) {
        return -1;
    } else if (x.size() > y.size()) {
        return 1;
    } else {
        return 0;
    }
}

#define DEFINE_OPERATORS(TYPE) \
        bool operator==(const TYPE& x, const TYPE& y) { return compare(x, y) == 0; } \
    bool operator!=(const TYPE& x, const TYPE& y) { return compare(x, y) != 0; } \
    bool operator< (const TYPE& x, const TYPE& y) { return compare(x, y) <  0; } \
    bool operator<=(const TYPE& x, const TYPE& y) { return compare(x, y) <= 0; } \
    bool operator> (const TYPE& x, const TYPE& y) { return compare(x, y) >  0; } \
    bool operator>=(const TYPE& x, const TYPE& y) { return compare(x, y) >= 0; }

DEFINE_OPERATORS(Bytes)
DEFINE_OPERATORS(BytesPiece)

}  // namespace sfz
