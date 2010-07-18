// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_READ_ITEM_HPP_
#define SFZ_READ_ITEM_HPP_

#include <sfz/ReadSource.hpp>

namespace sfz {

class ReadItem;

class ReadItem {
  public:
    template <typename T> ReadItem(T* t);

    inline void read_from(ReadSource in, size_t count);

  private:
    struct DispatchTable {
        void (*read_from)(void* target, ReadSource in, size_t count);
    };

    template <typename T> struct Dispatch;

    void* const _target;
    const DispatchTable* const _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
};

template <typename T>
void read(ReadSource in, T* items, size_t count = 1) {
    ReadItem(items).read_from(in, count);
}

template <typename T>
T read(ReadSource in) {
    T result;
    read(in, &result);
    return result;
}

// Implementation details follow.

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

#endif  // SFZ_READ_ITEM_HPP_
