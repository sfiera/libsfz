// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_PRINT_TARGET_HPP_
#define SFZ_PRINT_TARGET_HPP_

#include <stdlib.h>
#include <sfz/Rune.hpp>

namespace sfz {

class BytesPiece;
class String;
class StringPiece;

class PrintTarget {
  public:
    template <typename T> PrintTarget(T* t);

    inline void push(const StringPiece& string);
    inline void push(size_t num, Rune rune);

  private:
    struct DispatchTable {
        void (*push_string)(void* target, const StringPiece& string);
        void (*push_repeated_runes)(void* target, size_t num, Rune rune);
    };

    template <typename T> struct Dispatch;

    void* const _target;
    const DispatchTable* const _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
struct PrintTarget::Dispatch {
    static void push_string(void* target, const StringPiece& string) {
        reinterpret_cast<T*>(target)->push(string);
    }
    static void push_repeated_runes(void* target, size_t num, Rune rune) {
        reinterpret_cast<T*>(target)->push(num, rune);
    }
    static const DispatchTable table;
};

template <typename T>
const PrintTarget::DispatchTable PrintTarget::Dispatch<T>::table = {
    push_string,
    push_repeated_runes,
};

template <typename T>
PrintTarget::PrintTarget(T* t)
    : _target(t),
      _dispatch_table(&Dispatch<T>::table) { }

inline void PrintTarget::push(const StringPiece& string) {
    _dispatch_table->push_string(_target, string);
}

inline void PrintTarget::push(size_t num, Rune rune) {
    _dispatch_table->push_repeated_runes(_target, num, rune);
}

}  // namespace sfz

#endif  // SFZ_PRINT_TARGET_HPP_
