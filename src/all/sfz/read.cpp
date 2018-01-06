// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/read.hpp>

#include <pn/data>
#include <pn/file>
#include <sfz/bytes.hpp>
#include <sfz/range.hpp>

namespace sfz {

namespace {

inline void read_bytes_from(void* target, ReadSource in, size_t count) {
    in.shift(reinterpret_cast<uint8_t*>(target), count);
}

template <typename T>
inline void read_integers_from(void* target, ReadSource in, size_t count) {
    for (int i : range(count)) {
        Bytes bytes{sizeof(T), 0x00};
        in.shift(bytes.data(), bytes.size());
        pn::data_view{bytes.data(), static_cast<int>(bytes.size())}.open().read(
                &reinterpret_cast<T*>(target)[i]);
    }
}

}  // namespace

template <>
void ReadItem::Dispatch<bool>::read_from(void* target, ReadSource in, size_t count) {
    for (int i : range(count)) {
        uint8_t byte;
        in.shift(&byte, 1);
        reinterpret_cast<bool*>(target)[i] = (byte != 0);
    }
}

template <>
void ReadItem::Dispatch<char>::read_from(void* target, ReadSource in, size_t count) {
    read_bytes_from(target, in, count);
}

template <>
void ReadItem::Dispatch<int8_t>::read_from(void* target, ReadSource in, size_t count) {
    read_bytes_from(target, in, count);
}

template <>
void ReadItem::Dispatch<uint8_t>::read_from(void* target, ReadSource in, size_t count) {
    read_bytes_from(target, in, count);
}

template <>
void ReadItem::Dispatch<int16_t>::read_from(void* target, ReadSource in, size_t count) {
    read_integers_from<int16_t>(target, in, count);
}

template <>
void ReadItem::Dispatch<int32_t>::read_from(void* target, ReadSource in, size_t count) {
    read_integers_from<int32_t>(target, in, count);
}

template <>
void ReadItem::Dispatch<int64_t>::read_from(void* target, ReadSource in, size_t count) {
    read_integers_from<int64_t>(target, in, count);
}

template <>
void ReadItem::Dispatch<uint16_t>::read_from(void* target, ReadSource in, size_t count) {
    read_integers_from<uint16_t>(target, in, count);
}

template <>
void ReadItem::Dispatch<uint32_t>::read_from(void* target, ReadSource in, size_t count) {
    read_integers_from<uint32_t>(target, in, count);
}

template <>
void ReadItem::Dispatch<uint64_t>::read_from(void* target, ReadSource in, size_t count) {
    read_integers_from<uint64_t>(target, in, count);
}

}  // namespace sfz
