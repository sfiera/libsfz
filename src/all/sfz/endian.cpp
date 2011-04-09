// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/endian.hpp"

#include <arpa/inet.h>
#include <string.h>

namespace sfz {

namespace {

// There is no standard {hton,ntoh}ll() function, so we implement it here instead.
inline uint64_t swap_u64(uint64_t u64) {
    // Test the result of the htons() function.  In the worst case, this is probably a function
    // call resulting in a couple of bitwise operations, in which case this test should be cheap.
    // In the best case (on Darwin, for example), htons() is a macro, and an optimizing compiler
    // can eliminate the test entirely.
    if (htons(0x0001) == 0x0100) {
        return ((0ull + htonl(u64 & 0xffffffff)) << 32) | htonl(u64 >> 32);
    } else {
        return u64;
    }
}

// Convert from network to host byte order.  Overloaded for the integral types int{16,32,64}_t and
// uint{16,32,64}_t.
inline int16_t network_to_host(const int16_t& t) { return ntohs(t); }
inline uint16_t network_to_host(const uint16_t& t) { return ntohs(t); }
inline int32_t network_to_host(const int32_t& t) { return ntohl(t); }
inline uint32_t network_to_host(const uint32_t& t) { return ntohl(t); }
inline int64_t network_to_host(const int64_t& t) { return swap_u64(t); }
inline uint64_t network_to_host(const uint64_t& t) { return swap_u64(t); }

// Convert from host to network byte order.  Overloaded for the integral types int{16,32,64}_t and
// uint{16,32,64}_t.
inline int16_t host_to_network(const int16_t& t) { return htons(t); }
inline uint16_t host_to_network(const uint16_t& t) { return htons(t); }
inline int32_t host_to_network(const int32_t& t) { return htonl(t); }
inline uint32_t host_to_network(const uint32_t& t) { return htonl(t); }
inline int64_t host_to_network(const int64_t& t) { return swap_u64(t); }
inline uint64_t host_to_network(const uint64_t& t) { return swap_u64(t); }

}  // namespace

template <typename T>
NetworkBytes<T>::NetworkBytes() { }

template <typename T>
NetworkBytes<T>::NetworkBytes(T t) {
    set_value(t);
}

template <typename T>
size_t NetworkBytes<T>::size() const {
    return sizeof(T);
}

template <typename T>
uint8_t* NetworkBytes<T>::data() {
    return _data;
}

template <typename T>
const uint8_t* NetworkBytes<T>::data() const {
    return _data;
}

template <typename T>
T NetworkBytes<T>::value() const {
    return network_to_host(*reinterpret_cast<const T*>(_data));
}

template <typename T>
void NetworkBytes<T>::set_value(T t) {
    const T swapped = host_to_network(t);
    memcpy(_data, &swapped, size());
}

// Explicit template instantiation.
template class NetworkBytes<int16_t>;
template class NetworkBytes<uint16_t>;
template class NetworkBytes<int32_t>;
template class NetworkBytes<uint32_t>;
template class NetworkBytes<int64_t>;
template class NetworkBytes<uint64_t>;

}  // namespace sfz
