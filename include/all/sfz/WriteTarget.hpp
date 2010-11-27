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

    inline void push(const BytesPiece& bytes);
    inline void push(size_t num, uint8_t byte);

  private:
    struct DispatchTable {
        void (*push_bytes)(void* target, const BytesPiece& bytes);
        void (*push_repeated_bytes)(void* target, size_t num, uint8_t byte);
    };

    template <typename T> struct Dispatch;

    void* _target;
    const DispatchTable* _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
struct WriteTarget::Dispatch {
    static void push_bytes(void* target, const BytesPiece& bytes) {
        reinterpret_cast<T*>(target)->push(bytes);
    }
    static void push_repeated_bytes(void* target, size_t num, uint8_t byte) {
        reinterpret_cast<T*>(target)->push(num, byte);
    }
    static const DispatchTable table;
};

template <typename T>
const WriteTarget::DispatchTable WriteTarget::Dispatch<T>::table = {
    push_bytes,
    push_repeated_bytes,
};

template <typename T>
WriteTarget::WriteTarget(T* t)
    : _target(t),
      _dispatch_table(&Dispatch<T>::table) { }

inline void WriteTarget::push(const BytesPiece& bytes) {
    _dispatch_table->push_bytes(_target, bytes);
}

inline void WriteTarget::push(size_t num, uint8_t byte) {
    _dispatch_table->push_repeated_bytes(_target, num, byte);
}

}  // namespace sfz

#endif  // SFZ_WRITE_TARGET_HPP_
