// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_NETWORK_BYTES_HPP_
#define SFZ_NETWORK_BYTES_HPP_

#include <stdint.h>
#include <stdlib.h>

namespace sfz {

// Stores the bytes of an integer in network byte order.
template <typename T>
class NetworkBytes {
  public:
    // Creates an uninitialized array of sizeof(T).
    NetworkBytes();

    // Creates an array containing the bytes in `t` in network byte order.
    //
    // @param [in] t        The integer to represent the bytes of.
    explicit NetworkBytes(T t);

    size_t size() const;
    uint8_t* data();
    const uint8_t* data() const;

    T value() const;
    void set_value(T t);

  private:
    // An array of sizeof(T) bytes.
    uint8_t _data[sizeof(T)];
};

}  // namespace sfz

#endif  // SFZ_NETWORK_BYTES_HPP_
