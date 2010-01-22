// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_BINARY_READER_HPP_
#define SFZ_BINARY_READER_HPP_

#include <stdint.h>
#include <stdlib.h>
#include "sfz/Bytes.hpp"
#include "sfz/Macros.hpp"

namespace sfz {

// Interface for reading binary data.
//
// All primitive data read by BinaryReader subclasses is translated from network byte order to the
// host's byte order.
//
// In addition to being able to read primitive data types, BinaryReader objects can read data from
// complex types which conform to the binary-readable concept.  Conforming classes must expose a
// method `void read(BinaryReader* bin)`, which uses `bin` to read in a representation of the
// object.
class BinaryReader {
  public:
    BinaryReader();
    virtual ~BinaryReader();

    // Reads `count` values into `t`.
    //
    // The type of `t` must be one of the primitive types `bool`, `char`, or an integral type
    // `int<N>_t`, or a complex type which conforms to the binary-readable concept (see class
    // documentation).
    //
    // @param [out] t       An array of at least `count` objects of type `T` to read.
    // @param [in] count    The number of objects to read into `t`.
    // @throws Exception    If there are too few bytes to read.
    template <typename T>
    void read(T* t, size_t count = 1);

    // Reads `bytes` bytes and discards them.
    //
    // @param [in] bytes    The number of bytes to discard.
    // @throws Exception    If there are less than `bytes` bytes remaining to read.
    void discard(size_t bytes);

    // Indicates whether or not there is more data to be read.
    //
    // @returns             true iff there is no more data to read.
    virtual bool done() const = 0;

  protected:
    // Reads `count` bytes into `bytes`.
    //
    // This is the method which must be implemented by subclasses.
    //
    // @param [out] bytes   An array of at least `count` bytes to read.
    // @param [in] count    The number of bytes to read.
    // @throws Exception    If there are less than `count` bytes remaining to read.
    virtual void read_bytes(uint8_t* bytes, size_t count) = 0;

  private:
    // Reads `count` primitives into `t`.
    //
    // Identical to `read()`, except that it only supports primitives in its selection of `T`.
    template <typename T>
    void read_primitive(T* t, size_t count = 1);
};

// Reads binary data from an in-memory buffer.
//
// Uses an in-memory buffer of data as a source for BinaryReader.  The user of this object must not
// attempt to read more data from the buffer than it contains.
class BytesBinaryReader : public BinaryReader {
  public:
    // @param [in] data     The buffer to read from.
    explicit BytesBinaryReader(const BytesPiece& data);

    // Implementation of virtual BinaryReader methods.
    virtual bool done() const;

  protected:
    // Implementation of virtual BinaryReader methods.
    virtual void read_bytes(uint8_t* bytes, size_t count);

  private:
    // The start of the buffer to read from.  Shrinks towards the right as data is read.
    BytesPiece _data;

    DISALLOW_COPY_AND_ASSIGN(BytesBinaryReader);
};

// Implementation details follow.

// Specializations of `BinaryReader::read()` for primitive types.
template <> void BinaryReader::read<bool>(bool* b, size_t count);
template <> void BinaryReader::read<char>(char* c, size_t count);
template <> void BinaryReader::read<int8_t>(int8_t* i8, size_t count);
template <> void BinaryReader::read<uint8_t>(uint8_t* u8, size_t count);
template <> void BinaryReader::read<int16_t>(int16_t* i16, size_t count);
template <> void BinaryReader::read<uint16_t>(uint16_t* u16, size_t count);
template <> void BinaryReader::read<int32_t>(int32_t* i32, size_t count);
template <> void BinaryReader::read<uint32_t>(uint32_t* u32, size_t count);
template <> void BinaryReader::read<int64_t>(int64_t* i64, size_t count);
template <> void BinaryReader::read<uint64_t>(uint64_t* u64, size_t count);

// Default implementation of `BinaryReader::read()` for complex types.
template <typename T>
void BinaryReader::read(T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        t[i].read(this);
    }
}

}  // namespace sfz

#endif  // SFZ_BINARY_READER_HPP_
