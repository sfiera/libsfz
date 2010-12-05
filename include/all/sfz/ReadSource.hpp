// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_READ_SOURCE_HPP_
#define SFZ_READ_SOURCE_HPP_

#include <stdint.h>
#include <stdlib.h>

namespace sfz {

class ReadSource {
  public:
    template <typename T> ReadSource(T* target);

    inline bool empty() const;
    inline void shift(size_t size);
    inline void shift(uint8_t* data, size_t size);

  private:
    struct DispatchTable {
        bool (*empty)(const void* target);
        void (*shift)(void* target, size_t size);
        void (*shift_data)(void* target, uint8_t* data, size_t size);
    };

    template <typename T> struct Dispatch;

    void* _target;
    const DispatchTable* _dispatch_table;

    // ALLOW_COPY_AND_ASSIGN
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

}  // namespace sfz

#endif  // SFZ_READ_SOURCE_HPP_
