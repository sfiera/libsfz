// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/Bytes.hpp"

#include <algorithm>
#include "sfz/Encoding.hpp"
#include "sfz/String.hpp"

using std::max;

namespace sfz {

namespace {

const size_t kDefaultBytesSize = 16;

}  // namespace

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

Bytes::Bytes(const uint8_t* data, size_t size)
    : _data(new uint8_t[max(size, kDefaultBytesSize)]),
      _size(size),
      _capacity(max(size, kDefaultBytesSize)) {
    memcpy(_data.get(), data, size);
}

Bytes::Bytes(const StringPiece& string, const Encoding& encoding)
    : _data(new uint8_t[kDefaultBytesSize]),
      _size(0),
      _capacity(kDefaultBytesSize) {
    encoding.encode(string, this);
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

void Bytes::append(const BytesPiece& bytes) {
    append(bytes.data(), bytes.size());
}

void Bytes::append(const uint8_t* data, size_t size) {
    reserve(size + _size);
    memcpy(_data.get() + _size, data, size);
    _size += size;
}

void Bytes::append(const StringPiece& string, const Encoding& encoding) {
    encoding.encode(string, this);
}

void Bytes::append(size_t num, uint8_t byte) {
    reserve(num + _size);
    memset(_data.get() + _size, byte, num);
    _size += num;
}

void Bytes::assign(const BytesPiece& bytes) {
    assign(bytes.data(), bytes.size());
}

void Bytes::assign(const uint8_t* data, size_t size) {
    reserve(size);
    memcpy(_data.get(), data, size);
    _size = size;
}

void Bytes::assign(const StringPiece& string, const Encoding& encoding) {
    clear();
    encoding.encode(string, this);
}

void Bytes::assign(size_t num, uint8_t byte) {
    reserve(num);
    memset(_data.get(), byte, num);
    _size = num;
}

uint8_t Bytes::at(size_t loc) const {
    if (loc >= _size) {
        abort();
    }
    return _data.get()[loc];
}

void Bytes::clear() {
    _size = 0;
}

bool Bytes::empty() const {
    return _size == 0;
}

void Bytes::reserve(size_t capacity) {
    if (_capacity < capacity) {
        size_t new_capacity = _capacity * 2;
        while (new_capacity < capacity) {
            new_capacity *= 2;
        }
        scoped_array<uint8_t> new_data(new uint8_t[new_capacity]);
        memcpy(new_data.get(), _data.get(), _size);
        _data.swap(&new_data);
    }
}

void Bytes::resize(size_t size, uint8_t byte) {
    if (size < _size) {
        _size = size;
    } else {
        reserve(size);
        memset(_data.get() + _size, byte, size - _size);
    }
}

void Bytes::swap(Bytes* bytes) {
    _data.swap(&bytes->_data);
}

BytesPiece::BytesPiece()
    : _data(NULL),
      _size(0) { }

BytesPiece::BytesPiece(const Bytes& bytes)
    : _data(bytes.data()),
      _size(bytes.size()) { }

BytesPiece::BytesPiece(const uint8_t* data, size_t size)
    : _data(data),
      _size(size) { }

BytesPiece::BytesPiece(const_iterator begin, const_iterator end)
    : _data(begin._it),
      _size(end._it - begin._it) { }

const uint8_t* BytesPiece::data() const {
    return _data;
}

size_t BytesPiece::size() const {
    return _size;
}

uint8_t BytesPiece::at(size_t loc) const {
    if (loc >= _size) {
        abort();
    }
    return _data[loc];
}

bool BytesPiece::empty() const {
    return _size == 0;
}

BytesPiece BytesPiece::substr(size_t index) const {
    if (index > _size) {
        abort();
    }
    return BytesPiece(_data + index, _size - index);
}

BytesPiece BytesPiece::substr(size_t index, size_t size) const {
    if (index + size > _size) {
        abort();
    }
    return BytesPiece(_data + index, size);
}

BytesPiece::const_iterator BytesPiece::begin() const {
    return const_iterator(this, _data);
}

BytesPiece::const_iterator BytesPiece::end() const {
    return const_iterator(this, _data + _size);
}

// BytesPiece::const_iterator implementation.

BytesPiece::const_iterator::const_iterator(const BytesPiece* parent, const uint8_t* it)
    : _parent(parent),
      _it(it) { }

BytesPiece::const_iterator::value_type BytesPiece::const_iterator::operator*() const {
    return *_it;
}

BytesPiece::const_iterator::value_type BytesPiece::const_iterator::operator[](int n) const {
    return _it[n];
}

BytesPiece::const_iterator& BytesPiece::const_iterator::operator++() {
    ++_it;
    return *this;
}

BytesPiece::const_iterator BytesPiece::const_iterator::operator++(int) {
    return const_iterator(_parent, _it++);
}

BytesPiece::const_iterator& BytesPiece::const_iterator::operator--() {
    --_it;
    return *this;
}

BytesPiece::const_iterator BytesPiece::const_iterator::operator--(int) {
    return const_iterator(_parent, _it++);
}

BytesPiece::const_iterator BytesPiece::const_iterator::operator+(int n) {
    return const_iterator(_parent, _it + n);
}

BytesPiece::const_iterator& BytesPiece::const_iterator::operator+=(int n) {
    _it += n;
    return *this;
}

BytesPiece::const_iterator BytesPiece::const_iterator::operator-(int n) {
    return const_iterator(_parent, _it - n);
}

BytesPiece::const_iterator& BytesPiece::const_iterator::operator-=(int n) {
    _it -= n;
    return *this;
}

bool BytesPiece::const_iterator::operator==(const BytesPiece::const_iterator& it) {
    return _it == it._it;
}

bool BytesPiece::const_iterator::operator!=(const BytesPiece::const_iterator& it) {
    return _it != it._it;
}

}  // namespace sfz
