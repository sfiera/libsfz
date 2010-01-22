// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_BINARY_WRITER_HPP_
#define SFZ_BINARY_WRITER_HPP_

#include <stdint.h>
#include <stdlib.h>
#include "sfz/Bytes.hpp"
#include "sfz/Macros.hpp"

namespace sfz {

// Interface for writing binary data.
//
// All primitive data written by BinaryWriter subclasses is translated from the host's byte order
// to network byte order.
//
// In addition to being able to write primitive data types, BinaryWriter objects can write data
// from complex types which conform to the binary-writable concept.  Conforming classes must expose
// a method `void write(BinaryWriter* bin)`, which uses `bin` to write in a representation of the
// object.
class BinaryWriter {
  public:
    BinaryWriter();
    virtual ~BinaryWriter();

    // Writes `t`.
    //
    // The type of `t` must be one of the primitive types `bool`, `char`, or an integral type
    // `int<N>_t`, or a complex type which conforms to the binary-writable concept (see class
    // documentation).
    //
    // @param [in] t        The value to write.
    template <typename T>
    void write(const T& t);

    // Writes `count` values from `t`.
    //
    // The type of `t` must be one of the primitive types `bool`, `char`, or an integral type
    // `int<N>_t`, or a complex type which conforms to the binary-writable concept (see class
    // documentation).
    //
    // @param [in] t        An array of at least `count` objects of type `T` to write.
    // @param [in] count    The number of objects to write from `t`.
    template <typename T>
    void write(const T* t, size_t count);

    // Writes `bytes` bytes of blank data.
    //
    // @param [in] bytes    The number of bytes to pad with.
    void pad(size_t bytes);

  protected:
    // Reads `count` bytes into `bytes`.
    //
    // This is the method which must be implemented by subclasses.
    //
    // @param [in] bytes    An array of at least `count` bytes to write.
    // @param [in] count    The number of bytes to write.
    virtual void write_bytes(const uint8_t* bytes, size_t count) = 0;

  private:
    // Writes `count` primitives from `t`.
    //
    // Identical to `write()`, except that it only supports primitives in its selection of `T`.
    template <typename T>
    void write_primitive(const T* t, size_t count);
};

// Writes binary data to an in-memory buffer.
//
// Uses a Bytes object as a target for BinaryWriter.  Data which is written to the BinaryWriter
// will be appended to the object given.
class BytesBinaryWriter : public BinaryWriter {
  public:
    BytesBinaryWriter(Bytes* out)
            : _out(out) { }

    ~BytesBinaryWriter();

  protected:
    virtual void write_bytes(const uint8_t* bytes, size_t count);

  private:
    Bytes* _out;
};

// Implementation details follow.

// Specializations of `BinaryWriter::write()` for primitive types.
template <> void BinaryWriter::write<bool>(const bool* b, size_t count);
template <> void BinaryWriter::write<char>(const char* c, size_t count);
template <> void BinaryWriter::write<int8_t>(const int8_t* i8, size_t count);
template <> void BinaryWriter::write<uint8_t>(const uint8_t* u8, size_t count);
template <> void BinaryWriter::write<int16_t>(const int16_t* i16, size_t count);
template <> void BinaryWriter::write<uint16_t>(const uint16_t* u16, size_t count);
template <> void BinaryWriter::write<int32_t>(const int32_t* i32, size_t count);
template <> void BinaryWriter::write<uint32_t>(const uint32_t* u32, size_t count);
template <> void BinaryWriter::write<int64_t>(const int64_t* i64, size_t count);
template <> void BinaryWriter::write<uint64_t>(const uint64_t* u64, size_t count);

// Default implementation of `BinaryWriter::write()` for single instances of complex types.
// Forwards the call to the array case of `BinaryWriter::write()` as an array of size 1.
template <typename T>
void BinaryWriter::write(const T& t) {
    write(&t, 1);
}

// Default implementation of `BinaryWriter::write()` for arrays of complex types.
template <typename T>
void BinaryWriter::write(const T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        t[i].write(this);
    }
}

}  // namespace sfz

#endif  // SFZ_BINARY_WRITER_HPP_
