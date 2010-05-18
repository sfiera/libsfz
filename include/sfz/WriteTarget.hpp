// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_WRITE_TARGET_HPP_
#define SFZ_WRITE_TARGET_HPP_

#include <stdint.h>
#include <stdlib.h>

namespace sfz {

class StringPiece;
class BytesPiece;

class WriteTarget {
  public:
    template <typename T> WriteTarget(T* target);

    inline void append(const BytesPiece& bytes);
    inline void append(size_t num, uint8_t byte);

  private:
    struct DispatchTable {
        void (*append_bytes)(void* target, const BytesPiece& bytes);
        void (*append_repeated_bytes)(void* target, size_t num, uint8_t byte);
    };

    template <typename T> struct Dispatch;

    void* _target;
    const DispatchTable* _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
struct WriteTarget::Dispatch {
    static void append_bytes(void* target, const BytesPiece& bytes) {
        reinterpret_cast<T*>(target)->append(bytes);
    }
    static void append_repeated_bytes(void* target, size_t num, uint8_t byte) {
        reinterpret_cast<T*>(target)->append(num, byte);
    }
    static const DispatchTable table;
};

template <typename T>
const WriteTarget::DispatchTable WriteTarget::Dispatch<T>::table = {
    append_bytes,
    append_repeated_bytes,
};

template <typename T>
WriteTarget::WriteTarget(T* t)
    : _target(t),
      _dispatch_table(&Dispatch<T>::table) { }

inline void WriteTarget::append(const BytesPiece& bytes) {
    _dispatch_table->append_bytes(_target, bytes);
}

inline void WriteTarget::append(size_t num, uint8_t byte) {
    _dispatch_table->append_repeated_bytes(_target, num, byte);
}

}  // namespace sfz

#endif  // SFZ_WRITE_TARGET_HPP_
