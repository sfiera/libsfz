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

    inline void append(const char* string);
    inline void append(const String& string);
    inline void append(const StringPiece& string);
    inline void append(size_t num, Rune rune);

  private:
    struct DispatchTable {
        void (*append_c_string)(void* target, const char* string);
        void (*append_string)(void* target, const String& string);
        void (*append_string_piece)(void* target, const StringPiece& string);
        void (*append_runes)(void* target, size_t num, Rune rune);
    };

    template <typename T> struct Dispatch;

    void* const _target;
    const DispatchTable* const _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
struct PrintTarget::Dispatch {
    static void append_c_string(void* target, const char* string) {
        reinterpret_cast<T*>(target)->append(string);
    }
    static void append_string(void* target, const String& string) {
        reinterpret_cast<T*>(target)->append(string);
    }
    static void append_string_piece(void* target, const StringPiece& string) {
        reinterpret_cast<T*>(target)->append(string);
    }
    static void append_runes(void* target, size_t num, Rune rune) {
        reinterpret_cast<T*>(target)->append(num, rune);
    }
    static const DispatchTable table;
};

template <typename T>
const PrintTarget::DispatchTable PrintTarget::Dispatch<T>::table = {
    append_c_string,
    append_string,
    append_string_piece,
    append_runes,
};

template <typename T>
PrintTarget::PrintTarget(T* t)
    : _target(t),
      _dispatch_table(&Dispatch<T>::table) { }

inline void PrintTarget::append(const char* string) {
    _dispatch_table->append_c_string(_target, string);
}

inline void PrintTarget::append(const String& string) {
    _dispatch_table->append_string(_target, string);
}

inline void PrintTarget::append(const StringPiece& string) {
    _dispatch_table->append_string_piece(_target, string);
}

inline void PrintTarget::append(size_t num, Rune rune) {
    _dispatch_table->append_runes(_target, num, rune);
}

}  // namespace sfz

#endif  // SFZ_PRINT_TARGET_HPP_
