// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/bytes.hpp>

#include <string.h>
#include <algorithm>
#include <sfz/exception.hpp>
#include <sfz/foreach.hpp>
#include <sfz/macros.hpp>

namespace sfz {

using std::max;
using std::min;
using std::swap;
using std::unique_ptr;

namespace {

const size_t kDefaultBytesSize = 16;

}  // namespace

const size_t Bytes::npos;
const size_t BytesSlice::npos;

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

Bytes::Bytes(WriteItem item)
    : _data(new uint8_t[kDefaultBytesSize]),
      _size(0),
      _capacity(kDefaultBytesSize) {
    item.write_to(*this);
}

Bytes::Bytes(const uint8_t* data, size_t size)
    : _data(new uint8_t[max(size, kDefaultBytesSize)]),
      _size(size),
      _capacity(max(size, kDefaultBytesSize)) {
    memcpy(_data.get(), data, size);
}

Bytes::Bytes(size_t num, uint8_t byte)
    : _data(new uint8_t[max(num, kDefaultBytesSize)]),
      _size(num),
      _capacity(max(num, kDefaultBytesSize)) {
    memset(_data.get(), byte, num);
}

Bytes::~Bytes() { }

void Bytes::assign(Bytes&& bytes) {
    *this = std::move(bytes);
}

uint8_t* Bytes::data() {
    return _data.get();
}

const uint8_t* Bytes::data() const {
    return _data.get();
}

size_t Bytes::size() const {
    return _size;
}

void Bytes::push(const BytesSlice& bytes) {
    append(bytes.data(), bytes.size());
}

void Bytes::push(size_t num, uint8_t byte) {
    append(num, byte);
}

void Bytes::append(WriteItem item) {
    item.write_to(*this);
}

void Bytes::append(const uint8_t* data, size_t size) {
    reserve(size + _size);
    memcpy(_data.get() + _size, data, size);
    _size += size;
}

void Bytes::append(size_t num, uint8_t byte) {
    reserve(num + _size);
    memset(_data.get() + _size, byte, num);
    _size += num;
}

void Bytes::assign(WriteItem item) {
    clear();
    item.write_to(*this);
}

void Bytes::assign(const uint8_t* data, size_t size) {
    reserve(size);
    memcpy(_data.get(), data, size);
    _size = size;
}

void Bytes::assign(size_t num, uint8_t byte) {
    reserve(num);
    memset(_data.get(), byte, num);
    _size = num;
}

uint8_t Bytes::at(size_t loc) const {
    return slice().at(loc);
}

size_t Bytes::find(uint8_t byte) const {
    return slice().find(byte);
}

size_t Bytes::find(const BytesSlice& bytes) const {
    return slice().find(bytes);
}

size_t Bytes::rfind(uint8_t byte) const {
    return slice().rfind(byte);
}

size_t Bytes::rfind(const BytesSlice& bytes) const {
    return slice().rfind(bytes);
}

void Bytes::clear() {
    _size = 0;
}

bool Bytes::empty() const {
    return _size == 0;
}

BytesSlice Bytes::slice() const {
    return *this;
}

BytesSlice Bytes::slice(size_t index) const {
    return slice().slice(index);
}

BytesSlice Bytes::slice(size_t index, size_t size) const {
    return slice().slice(index, size);
}

void Bytes::replace(size_t index, size_t num, const BytesSlice& bytes) {
    Bytes tail(slice(index + num));
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
        unique_ptr<uint8_t[]> new_data(new uint8_t[new_capacity]);
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

BytesSlice::BytesSlice()
    : _data(NULL),
      _size(0) { }

BytesSlice::BytesSlice(const Bytes& bytes)
    : _data(bytes.data()),
      _size(bytes.size()) { }

BytesSlice::BytesSlice(const char* data)
    : _data(reinterpret_cast<const uint8_t*>(data)),
      _size(strlen(data)) { }

BytesSlice::BytesSlice(const uint8_t* data, size_t size)
    : _data(data),
      _size(size) { }

const uint8_t* BytesSlice::data() const {
    return _data;
}

size_t BytesSlice::size() const {
    return _size;
}

uint8_t BytesSlice::at(size_t loc) const {
    if (loc >= _size) {
        throw Exception("out-of-bounds");
    }
    return _data[loc];
}

size_t BytesSlice::find(uint8_t byte) const {
    SFZ_FOREACH(size_type i, range(_size), {
        if (at(i) == byte) {
            return i;
        }
    });
    return BytesSlice::npos;
}

size_t BytesSlice::find(const BytesSlice& bytes) const {
    if (_size < bytes.size()) {
        return BytesSlice::npos;
    }
    SFZ_FOREACH(size_type i, range(_size - bytes.size() + 1), {
        if (slice(i, bytes.size()) == bytes) {
            return i;
        }
    });
    return BytesSlice::npos;
}

size_t BytesSlice::rfind(uint8_t byte) const {
    SFZ_FOREACH(size_type i, range(_size), {
        if (at(_size - i - 1) == byte) {
            return _size - i - 1;
        }
    });
    return BytesSlice::npos;
}

size_t BytesSlice::rfind(const BytesSlice& bytes) const {
    if (_size < bytes.size()) {
        return BytesSlice::npos;
    }
    SFZ_FOREACH(size_type i, range(_size - bytes.size() + 1), {
        if (slice(_size - bytes.size() - i, bytes.size()) == bytes) {
            return _size - bytes.size() - i;
        }
    });
    return BytesSlice::npos;
}

bool BytesSlice::empty() const {
    return _size == 0;
}

BytesSlice BytesSlice::slice() const {
    return *this;
}

BytesSlice BytesSlice::slice(size_t index) const {
    if (index > _size) {
        throw Exception("out-of-bounds");
    }
    return BytesSlice(_data + index, _size - index);
}

BytesSlice BytesSlice::slice(size_t index, size_t size) const {
    if (index + size > _size) {
        throw Exception("out-of-bounds");
    }
    return BytesSlice(_data + index, size);
}

void BytesSlice::shift(size_t size) {
    if (size > _size) {
        throw Exception("out-of-bounds");
    }
    _data += size;
    _size -= size;
}

void BytesSlice::shift(uint8_t* data, size_t size) {
    shift(size);
    memcpy(data, _data - size, size);
}

void swap(Bytes& x, Bytes& y) {
    swap(x._data, y._data);
    swap(x._size, y._size);
    swap(x._capacity, y._capacity);
}

void swap(BytesSlice& x, BytesSlice& y) {
    swap(x._data, y._data);
    swap(x._size, y._size);
}

bool operator==(const BytesSlice& x, const BytesSlice& y) {
    return (x.size() == y.size())
        && (memcmp(x.data(), y.data(), x.size()) == 0);
}

bool operator<(const BytesSlice& x, const BytesSlice& y) {
    if (x.size() < y.size()) {
        return memcmp(x.data(), y.data(), x.size()) <= 0;
    } else {
        return memcmp(x.data(), y.data(), y.size()) < 0;
    }
}

}  // namespace sfz
