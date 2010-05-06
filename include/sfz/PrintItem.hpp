// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_PRINT_ITEM_HPP_
#define SFZ_PRINT_ITEM_HPP_

#include "sfz/Macros.hpp"
#include "sfz/PrintTarget.hpp"
#include "sfz/String.hpp"
#include "sfz/ReferenceCounted.hpp"

namespace sfz {

class PrintItem;

class PrintItem {
  public:
    // Empty formatter.  Prints an empty string.
    PrintItem();

    // String formatter.  Prints an ASCII string.
    PrintItem(const char* string);

    // Boolean formatter.  Prints either "true" or "false".
    PrintItem(bool b);

    // Character formatter.  Prints the character, raw.
    PrintItem(char ch);

    // Integer formatters.  Prints the number in digits.
    PrintItem(signed char i, int base = 10, int min_width = 1);
    PrintItem(signed short i, int base = 10, int min_width = 1);
    PrintItem(signed int i, int base = 10, int min_width = 1);
    PrintItem(signed long i, int base = 10, int min_width = 1);
    PrintItem(signed long long i, int base = 10, int min_width = 1);

    PrintItem(unsigned char i, int base = 10, int min_width = 1);
    PrintItem(unsigned short i, int base = 10, int min_width = 1);
    PrintItem(unsigned int i, int base = 10, int min_width = 1);
    PrintItem(unsigned long i, int base = 10, int min_width = 1);
    PrintItem(unsigned long long i, int base = 10, int min_width = 1);

    // Floating-point formatters.  Prints using the "%f" printf() code family.
    PrintItem(float f);
    PrintItem(double d);

    // Pointer formatter.  Prints the integral value of the pointer in hexadecimal, padded to an
    // architecture-dependent width.  The templated form of the constructor will forwards non-void
    // pointers to this call.
    PrintItem(const void* pointer);

    // Templated formatter.
    template <typename T> PrintItem(const T& object);

    class Impl : public ReferenceCounted {
      public:
        virtual ~Impl();
        virtual void print_to(PrintTarget out) const = 0;
    };
    static PrintItem make(Impl* printer);

    void print_to(PrintTarget out) const;

  private:
    RefPtr<const Impl> _printer;

    // ALLOW_COPY_AND_ASSIGN
};

// Implementation details follow.

}  // namespace sfz

namespace sfz_adl {

template <typename T>
inline void adl_print_to(::sfz::PrintTarget out, const T& object) {
    print_to(out, object);
}

}  // sfz_adl

namespace sfz {

template <typename T>
class TemplatedItemPrinter : public PrintItem::Impl {
  public:
    TemplatedItemPrinter(const T& object)
        : _object(object) { }

    virtual void print_to(PrintTarget out) const {
        ::sfz_adl::adl_print_to(out, _object);
    }

    static PrintItem make(const T& object) {
        return PrintItem::make(new TemplatedItemPrinter(object));
    }

  private:
    const T& _object;

    DISALLOW_COPY_AND_ASSIGN(TemplatedItemPrinter);
};

template <typename T>
class TemplatedItemPrinter<T*> {
  public:
    static PrintItem make(const T* object) {
        const void* pointer = object;
        return PrintItem(pointer);
    }

    DISALLOW_COPY_AND_ASSIGN(TemplatedItemPrinter);
};

template <typename T>
PrintItem::PrintItem(const T& object) {
    *this = TemplatedItemPrinter<T>::make(object);
}

}  // namespace sfz

#endif  // SFZ_PRINT_ITEM_HPP_
