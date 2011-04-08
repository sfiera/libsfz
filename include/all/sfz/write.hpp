// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_WRITE_HPP_
#define SFZ_WRITE_HPP_

#include <stdint.h>
#include <stdlib.h>

namespace sfz {

class BytesSlice;
class WriteTarget;

template <typename T>
void write(WriteTarget out, const T& item);

template <typename T>
void write(WriteTarget out, const T* items, size_t count);

class WriteTarget {
  public:
    template <typename T> WriteTarget(T* target);

    inline void push(const BytesSlice& bytes);
    inline void push(size_t num, uint8_t byte);

  private:
    struct DispatchTable;
    template <typename T> struct Dispatch;

    void* _target;
    const DispatchTable* _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

class WriteItem {
  public:
    template <typename T> WriteItem(const T& t);
    template <typename T> WriteItem(const T* t, size_t size);

    inline void write_to(WriteTarget out) const;

  private:
    struct DispatchTable;
    template <typename T> struct Dispatch;

    const void* _target;
    const size_t _size;
    const DispatchTable* _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
void write(WriteTarget out, const T& item) {
    WriteItem(item).write_to(out);
}

template <typename T>
void write(WriteTarget out, const T* items, size_t count) {
    WriteItem(items, count).write_to(out);
}

struct WriteTarget::DispatchTable {
    void (*push_bytes)(void* target, const BytesSlice& bytes);
    void (*push_repeated_bytes)(void* target, size_t num, uint8_t byte);
};

template <typename T>
struct WriteTarget::Dispatch {
    static void push_bytes(void* target, const BytesSlice& bytes) {
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

inline void WriteTarget::push(const BytesSlice& bytes) {
    _dispatch_table->push_bytes(_target, bytes);
}

inline void WriteTarget::push(size_t num, uint8_t byte) {
    _dispatch_table->push_repeated_bytes(_target, num, byte);
}

// Implementation details follow.

}  // namespace sfz

namespace sfz_adl {

template <typename T>
inline void adl_write_to(::sfz::WriteTarget out, const T& t) {
    write_to(out, t);
}

}  // sfz_adl

namespace sfz {

struct WriteItem::DispatchTable {
    void (*write_to)(const void* target, WriteTarget out, size_t count);
};

template <typename T>
struct WriteItem::Dispatch {
    static void write_to(const void* target, WriteTarget out, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            ::sfz_adl::adl_write_to(out, reinterpret_cast<const T*>(target)[i]);
        }
    }
    static const DispatchTable table;
};

#define SFZ_WRITE_ITEM_SPECIALIZE(TYPE) \
    template <> void WriteItem::Dispatch<TYPE>::write_to( \
            const void* target, WriteTarget out, size_t count);
SFZ_WRITE_ITEM_SPECIALIZE(bool);
SFZ_WRITE_ITEM_SPECIALIZE(char);
SFZ_WRITE_ITEM_SPECIALIZE(int8_t);
SFZ_WRITE_ITEM_SPECIALIZE(int16_t);
SFZ_WRITE_ITEM_SPECIALIZE(int32_t);
SFZ_WRITE_ITEM_SPECIALIZE(int64_t);
SFZ_WRITE_ITEM_SPECIALIZE(uint8_t);
SFZ_WRITE_ITEM_SPECIALIZE(uint16_t);
SFZ_WRITE_ITEM_SPECIALIZE(uint32_t);
SFZ_WRITE_ITEM_SPECIALIZE(uint64_t);
#undef SFZ_WRITE_ITEM_SPECIALIZE

template <typename T>
const WriteItem::DispatchTable WriteItem::Dispatch<T>::table = {
    write_to,
};

template <typename T>
WriteItem::WriteItem(const T& t)
    : _target(&t),
      _size(1),
      _dispatch_table(&Dispatch<T>::table) { }

template <typename T>
WriteItem::WriteItem(const T* t, size_t size)
    : _target(t),
      _size(size),
      _dispatch_table(&Dispatch<T>::table) { }

inline void WriteItem::write_to(WriteTarget out) const {
    _dispatch_table->write_to(_target, out, _size);
}

}  // namespace sfz

#endif  // SFZ_WRITE_HPP_
