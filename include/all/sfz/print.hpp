// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_PRINT_HPP_
#define SFZ_PRINT_HPP_

#include <stdint.h>
#include <stdlib.h>

namespace sfz {

class PrintTarget;
class StringSlice;

// A type capable of representing any valid Unicode code point.
typedef uint32_t Rune;

template <typename T>
void print(PrintTarget out, const T& item);

class PrintTarget {
  public:
    PrintTarget(const PrintTarget& t);
    template <typename T> PrintTarget(T& t);

    inline void push(const StringSlice& string);
    inline void push(size_t num, Rune rune);

  private:
    struct DispatchTable;
    template <typename T> struct Dispatch;

    void* const _target;
    const DispatchTable* const _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

class PrintItem {
  public:
    PrintItem();
    template <typename T> PrintItem(const T& object);

    void print_to(PrintTarget out) const;

  private:
    struct DispatchTable;
    template <typename T> struct Dispatch;

    const void* const _target;
    const DispatchTable* const _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
void print(PrintTarget out, const T& item) {
    PrintItem(item).print_to(out);
}

struct PrintTarget::DispatchTable {
    void (*push_string)(void* target, const StringSlice& string);
    void (*push_repeated_runes)(void* target, size_t num, Rune rune);
};

template <typename T>
struct PrintTarget::Dispatch {
    static void push_string(void* target, const StringSlice& string) {
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

inline PrintTarget::PrintTarget(const PrintTarget& other)
    : _target(other._target),
      _dispatch_table(other._dispatch_table) { }

template <typename T>
PrintTarget::PrintTarget(T& t)
    : _target(&t),
      _dispatch_table(&Dispatch<T>::table) { }

inline void PrintTarget::push(const StringSlice& string) {
    _dispatch_table->push_string(_target, string);
}

inline void PrintTarget::push(size_t num, Rune rune) {
    _dispatch_table->push_repeated_runes(_target, num, rune);
}

struct PrintItem::DispatchTable {
    void (*print_to)(const void* target, PrintTarget out);
};

}  // namespace sfz

namespace sfz_adl {

template <typename T>
inline void adl_print_to(::sfz::PrintTarget out, const T& object) {
    print_to(out, object);
}

}  // sfz_adl

namespace sfz {

template <typename T>
struct PrintItem::Dispatch {
    static void print_to(const void* target, PrintTarget out) {
        ::sfz_adl::adl_print_to(out, *reinterpret_cast<const T*>(target));
    }
    static const DispatchTable table;
};

template <>
struct PrintItem::Dispatch<const char*> {
    static void print_to(const void* target, PrintTarget out);
    static const DispatchTable table;
};

template <>
struct PrintItem::Dispatch<const void*> {
    static void print_to(const void* target, PrintTarget out);
    static const DispatchTable table;
};

template <typename T>
struct PrintItem::Dispatch<T*> : public Dispatch<const void*> { };

#define SFZ_PRINT_ITEM_SPECIALIZE(TYPE) \
    template <> void PrintItem::Dispatch<TYPE>::print_to(const void* target, PrintTarget out);
SFZ_PRINT_ITEM_SPECIALIZE(void);
SFZ_PRINT_ITEM_SPECIALIZE(bool);
SFZ_PRINT_ITEM_SPECIALIZE(char);
SFZ_PRINT_ITEM_SPECIALIZE(signed char);
SFZ_PRINT_ITEM_SPECIALIZE(signed short);
SFZ_PRINT_ITEM_SPECIALIZE(signed int);
SFZ_PRINT_ITEM_SPECIALIZE(signed long);
SFZ_PRINT_ITEM_SPECIALIZE(signed long long);
SFZ_PRINT_ITEM_SPECIALIZE(unsigned char);
SFZ_PRINT_ITEM_SPECIALIZE(unsigned short);
SFZ_PRINT_ITEM_SPECIALIZE(unsigned int);
SFZ_PRINT_ITEM_SPECIALIZE(unsigned long);
SFZ_PRINT_ITEM_SPECIALIZE(unsigned long long);
SFZ_PRINT_ITEM_SPECIALIZE(float);
SFZ_PRINT_ITEM_SPECIALIZE(double);
SFZ_PRINT_ITEM_SPECIALIZE(const char[]);
#undef SFZ_PRINT_ITEM_SPECIALIZE

template <int size>
struct PrintItem::Dispatch<const char[size]> : public Dispatch<const char[]> { };

template <typename T>
const PrintItem::DispatchTable PrintItem::Dispatch<T>::table = {
    print_to,
};

inline PrintItem::PrintItem()
    : _target(NULL),
      _dispatch_table(&Dispatch<void>::table) { }

template <typename T>
PrintItem::PrintItem(const T& t)
    : _target(&t),
      _dispatch_table(&Dispatch<T>::table) { }

inline void PrintItem::print_to(PrintTarget out) const {
    _dispatch_table->print_to(_target, out);
}

}  // namespace sfz

#endif  // SFZ_PRINT_HPP_
