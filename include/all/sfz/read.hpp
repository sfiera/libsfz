// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_READ_HPP_
#define SFZ_READ_HPP_

#include <stdint.h>
#include <stdlib.h>

namespace sfz {

class ReadSource;

template <typename T>
void read(ReadSource in, T* items, size_t count = 1);

template <typename T>
T read(ReadSource in);

class ReadSource {
  public:
    template <typename T> ReadSource(T* target);

    inline bool empty() const;
    inline void shift(size_t size);
    inline void shift(uint8_t* data, size_t size);

  private:
    struct DispatchTable;
    template <typename T> struct Dispatch;

    void* _target;
    const DispatchTable* _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

class ReadItem {
  public:
    template <typename T> ReadItem(T* t);

    inline void read_from(ReadSource in, size_t count);

  private:
    struct DispatchTable;
    template <typename T> struct Dispatch;

    void* const _target;
    const DispatchTable* const _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

// Implementation details follow.

template <typename T>
void read(ReadSource in, T* items, size_t count) {
    ReadItem(items).read_from(in, count);
}

template <typename T>
T read(ReadSource in) {
    T result;
    read(in, &result);
    return result;
}

struct ReadSource::DispatchTable {
    bool (*empty)(const void* target);
    void (*shift)(void* target, size_t size);
    void (*shift_data)(void* target, uint8_t* data, size_t size);
};

template <typename T>
struct ReadSource::Dispatch {
    static bool empty(const void* target) {
        return reinterpret_cast<const T*>(target)->empty();
    }
    static void shift(void* target, size_t size) {
        reinterpret_cast<T*>(target)->shift(size);
    }
    static void shift_data(void* target, uint8_t* data, size_t size) {
        reinterpret_cast<T*>(target)->shift(data, size);
    }
    static const DispatchTable table;
};

template <typename T>
const ReadSource::DispatchTable ReadSource::Dispatch<T>::table = {
    empty,
    shift,
    shift_data,
};

template <typename T>
ReadSource::ReadSource(T* t)
    : _target(t),
      _dispatch_table(&Dispatch<T>::table) { }

inline bool ReadSource::empty() const {
    return _dispatch_table->empty(_target);
}

inline void ReadSource::shift(size_t size) {
    _dispatch_table->shift(_target, size);
}

inline void ReadSource::shift(uint8_t* data, size_t size) {
    _dispatch_table->shift_data(_target, data, size);
}

struct ReadItem::DispatchTable {
    void (*read_from)(void* target, ReadSource in, size_t count);
};

}  // namespace sfz

namespace sfz_adl {

template <typename T>
inline void adl_read_from(::sfz::ReadSource in, T* t) {
    read_from(in, t);
}

}  // sfz_adl

namespace sfz {

template <typename T>
struct ReadItem::Dispatch {
    static void read_from(void* target, ReadSource in, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            ::sfz_adl::adl_read_from(in, reinterpret_cast<T*>(target) + i);
        }
    }
    static const DispatchTable table;
};

#define SFZ_READ_ITEM_SPECIALIZE(TYPE) \
    template <> void ReadItem::Dispatch<TYPE>::read_from( \
            void* target, ReadSource in, size_t count);
SFZ_READ_ITEM_SPECIALIZE(bool);
SFZ_READ_ITEM_SPECIALIZE(char);
SFZ_READ_ITEM_SPECIALIZE(int8_t);
SFZ_READ_ITEM_SPECIALIZE(int16_t);
SFZ_READ_ITEM_SPECIALIZE(int32_t);
SFZ_READ_ITEM_SPECIALIZE(int64_t);
SFZ_READ_ITEM_SPECIALIZE(uint8_t);
SFZ_READ_ITEM_SPECIALIZE(uint16_t);
SFZ_READ_ITEM_SPECIALIZE(uint32_t);
SFZ_READ_ITEM_SPECIALIZE(uint64_t);
#undef SFZ_READ_ITEM_SPECIALIZE

template <typename T>
const ReadItem::DispatchTable ReadItem::Dispatch<T>::table = {
    read_from,
};

template <typename T>
ReadItem::ReadItem(T* t)
    : _target(t),
      _dispatch_table(&Dispatch<T>::table) { }

inline void ReadItem::read_from(ReadSource in, size_t count) {
    _dispatch_table->read_from(_target, in, count);
}

}  // namespace sfz

#endif  // SFZ_READ_HPP_