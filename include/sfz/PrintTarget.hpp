// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_PRINT_TARGET_HPP_
#define SFZ_PRINT_TARGET_HPP_

#include <stdlib.h>
#include "sfz/Rune.hpp"

namespace sfz {

class BytesPiece;
class Encoding;
class String;
class StringPiece;

class PrintTarget {
  public:
    template <typename T>
    PrintTarget(T* target) {
        new (impl_) Derived<T>(target);
    }

    inline void append(const String& string) { base()->append(string); }
    inline void append(const StringPiece& string) { base()->append(string); }
    inline void append(const BytesPiece& bytes, const Encoding& encoding) {
        base()->append(bytes, encoding); }
    inline void append(size_t num, Rune rune) { base()->append(num, rune); }

  private:
    struct Base {
        virtual ~Base() { }
        virtual void append(const String& string) = 0;
        virtual void append(const StringPiece& string) = 0;
        virtual void append(const BytesPiece& bytes, const Encoding& encoding) = 0;
        virtual void append(size_t num, Rune rune) = 0;
    };

    template <typename T>
    struct Derived : public Base {
        Derived(T* target)
            : target_(target) { }

        virtual void append(const String& string) { target_->append(string); }
        virtual void append(const StringPiece& string) { target_->append(string); }
        virtual void append(const BytesPiece& bytes, const Encoding& encoding) {
            target_->append(bytes, encoding); }
        virtual void append(size_t num, Rune rune) { target_->append(num, rune); }

        T* const target_;
    };

    inline Base* base() {
        return reinterpret_cast<Base*>(&impl_);
    };

    intptr_t impl_[2];

    // ALLOW_COPY_AND_ASSIGN
};

}  // namespace sfz

#endif  // SFZ_PRINT_TARGET_HPP_
