// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/BinaryReader.hpp"

#include <string.h>
#include <libkern/OSByteOrder.h>
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/NetworkBytes.hpp"
#include "sfz/Range.hpp"

namespace sfz {

BinaryReader::BinaryReader() { }

BinaryReader::~BinaryReader() { }

template <typename T>
void BinaryReader::read_primitive(T* t, size_t count) {
    foreach (i, range(count)) {
        NetworkBytes<T> bytes;
        read_bytes(bytes.mutable_data(), bytes.size());
        t[i] = bytes.value();
    }
}

template <>
void BinaryReader::read<bool>(bool* b, size_t count) {
    foreach (i, range(count)) {
        uint8_t c;
        read_bytes(&c, 1);
        b[i] = c;
    }
}

template <>
void BinaryReader::read<char>(char* c, size_t count) {
    read_bytes(reinterpret_cast<uint8_t*>(c), count);
}

template <>
void BinaryReader::read<int8_t>(int8_t* i8, size_t count) {
    read_bytes(reinterpret_cast<uint8_t*>(i8), count);
}

template <>
void BinaryReader::read<uint8_t>(uint8_t* u8, size_t count) {
    read_bytes(u8, count);
}

template <>
void BinaryReader::read<int16_t>(int16_t* i16, size_t count) {
    read_primitive(i16, count);
}

template <>
void BinaryReader::read<uint16_t>(uint16_t* u16, size_t count) {
    read_primitive(u16, count);
}

template <>
void BinaryReader::read<int32_t>(int32_t* i32, size_t count) {
    read_primitive(i32, count);
}

template <>
void BinaryReader::read<uint32_t>(uint32_t* u32, size_t count) {
    read_primitive(u32, count);
}

template <>
void BinaryReader::read<int64_t>(int64_t* i64, size_t count) {
    read_primitive(i64, count);
}

template <>
void BinaryReader::read<uint64_t>(uint64_t* u64, size_t count) {
    read_primitive(u64, count);
}

void BinaryReader::discard(size_t bytes) {
    foreach (i, range(bytes)) {
        uint8_t c;
        read_bytes(&c, 1);
    }
}

BytesBinaryReader::BytesBinaryReader(const BytesPiece& data)
    : _data(data) { }

bool BytesBinaryReader::done() const {
    return _data.empty();
}

void BytesBinaryReader::read_bytes(uint8_t* bytes, size_t count) {
    if (count > _data.size()) {
        throw Exception("underflow in BytesBinaryReader::read_bytes()");
    }
    memcpy(bytes, _data.data(), count);
    _data = _data.substr(count);
}

}  // namespace sfz
