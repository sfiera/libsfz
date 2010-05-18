// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_PRINT_ITEM_HPP_
#define SFZ_PRINT_ITEM_HPP_

#include "sfz/PrintTarget.hpp"

namespace sfz {

class PrintItem;

class PrintItem {
  public:
    PrintItem();
    template <typename T> PrintItem(const T& object);

    void print_to(PrintTarget out) const;

  private:
    struct DispatchTable {
        void (*print_to)(const void* target, PrintTarget out);
    };

    template <typename T> struct Dispatch;

    const void* const _target;
    const DispatchTable* const _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
void print(PrintTarget out, const T& item) {
    PrintItem(item).print_to(out);
}

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
struct PrintItem::Dispatch {
    static void print_to(const void* target, PrintTarget out) {
        ::sfz_adl::adl_print_to(out, *reinterpret_cast<const T*>(target));
    }
    static const DispatchTable table;
};

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
SFZ_PRINT_ITEM_SPECIALIZE(const char*);
SFZ_PRINT_ITEM_SPECIALIZE(const void*);
#undef SFZ_PRINT_ITEM_SPECIALIZE

template <typename T>
struct PrintItem::Dispatch<T*> : public Dispatch<const void*> { };

template <typename T, int size>
struct PrintItem::Dispatch<T[size]> : public Dispatch<T*> { };

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

#endif  // SFZ_PRINT_ITEM_HPP_
