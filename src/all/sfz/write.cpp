// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/write.hpp>

#include <sfz/bytes.hpp>
#include <sfz/endian.hpp>
#include <sfz/range.hpp>

namespace sfz {

namespace {

inline void write_bytes_to(const void* target, WriteTarget out, size_t count) {
    out.push(BytesSlice(reinterpret_cast<const uint8_t*>(target), count));
}

template <typename T>
inline void write_integers_to(const void* target, WriteTarget out, size_t count) {
    for (size_t i : range(count)) {
        NetworkBytes<T> bytes(reinterpret_cast<const T*>(target)[i]);
        out.push(BytesSlice(bytes.data(), bytes.size()));
    }
}

}  // namespace

template <>
void WriteItem::Dispatch<bool>::write_to(const void* target, WriteTarget out, size_t count) {
    for (size_t i : range(count)) {
        if (reinterpret_cast<const bool*>(target)[i]) {
            out.push(1, 0x01);
        } else {
            out.push(1, 0x00);
        }
    }
}

template <>
void WriteItem::Dispatch<char>::write_to(const void* target, WriteTarget out, size_t count) {
    write_bytes_to(target, out, count);
}

template <>
void WriteItem::Dispatch<int8_t>::write_to(const void* target, WriteTarget out, size_t count) {
    write_bytes_to(target, out, count);
}

template <>
void WriteItem::Dispatch<uint8_t>::write_to(const void* target, WriteTarget out, size_t count) {
    write_bytes_to(target, out, count);
}

template <>
void WriteItem::Dispatch<int16_t>::write_to(const void* target, WriteTarget out, size_t count) {
    write_integers_to<int16_t>(target, out, count);
}

template <>
void WriteItem::Dispatch<int32_t>::write_to(const void* target, WriteTarget out, size_t count) {
    write_integers_to<int32_t>(target, out, count);
}

template <>
void WriteItem::Dispatch<int64_t>::write_to(const void* target, WriteTarget out, size_t count) {
    write_integers_to<int64_t>(target, out, count);
}

template <>
void WriteItem::Dispatch<uint16_t>::write_to(const void* target, WriteTarget out, size_t count) {
    write_integers_to<uint16_t>(target, out, count);
}

template <>
void WriteItem::Dispatch<uint32_t>::write_to(const void* target, WriteTarget out, size_t count) {
    write_integers_to<uint32_t>(target, out, count);
}

template <>
void WriteItem::Dispatch<uint64_t>::write_to(const void* target, WriteTarget out, size_t count) {
    write_integers_to<uint64_t>(target, out, count);
}

}  // namespace sfz
