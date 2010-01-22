// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/BinaryWriter.hpp"

#include <string.h>
#include <libkern/OSByteOrder.h>
#include "sfz/Foreach.hpp"
#include "sfz/NetworkBytes.hpp"
#include "sfz/Range.hpp"

namespace sfz {

BinaryWriter::BinaryWriter() { }

BinaryWriter::~BinaryWriter() { }

template <typename T>
void BinaryWriter::write_primitive(const T* t, size_t count) {
    foreach (i, range(count)) {
        NetworkBytes<T> bytes(t[i]);
        write_bytes(bytes.data(), bytes.size());
    }
}

template <> void BinaryWriter::write<bool>(const bool* b, size_t count) {
    foreach (i, range(count)) {
        uint8_t c = b[i];
        write_bytes(&c, 1);
    }
}

template <> void BinaryWriter::write<char>(const char* c, size_t count) {
    write_bytes(reinterpret_cast<const uint8_t*>(c), count);
}

template <> void BinaryWriter::write<int8_t>(const int8_t* i8, size_t count) {
    write_bytes(reinterpret_cast<const uint8_t*>(i8), count);
}

template <> void BinaryWriter::write<uint8_t>(const uint8_t* uc, size_t count) {
    write_bytes(uc, count);
}

template <> void BinaryWriter::write<int16_t>(const int16_t* i16, size_t count) {
    write_primitive(i16, count);
}

template <> void BinaryWriter::write<uint16_t>(const uint16_t* u16, size_t count) {
    write_primitive(u16, count);
}

template <> void BinaryWriter::write<int32_t>(const int32_t* i32, size_t count) {
    write_primitive(i32, count);
}

template <> void BinaryWriter::write<uint32_t>(const uint32_t* u32, size_t count) {
    write_primitive(u32, count);
}

template <> void BinaryWriter::write<int64_t>(const int64_t* i64, size_t count) {
    write_primitive(i64, count);
}

template <> void BinaryWriter::write<uint64_t>(const uint64_t* u64, size_t count) {
    write_primitive(u64, count);
}

void BinaryWriter::pad(size_t bytes) {
    uint8_t c = '\0';
    foreach (i, range(bytes)) {
        write_bytes(&c, 1);
    }
}

BytesBinaryWriter::~BytesBinaryWriter() { }

void BytesBinaryWriter::write_bytes(const uint8_t* bytes, size_t count) {
    _out->append(bytes, count);
}

}  // namespace sfz
