// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/NetworkBytes.hpp"

#include <string.h>
#include <libkern/OSByteOrder.h>

namespace sfz {

namespace {

// Convert from network to host byte order.  Overloaded for the integral types int{16,32,64}_t and
// uint{16,32,64}_t.
inline int16_t network_to_host(const int16_t& t) { return OSSwapBigToHostInt16(t); }
inline uint16_t network_to_host(const uint16_t& t) { return OSSwapBigToHostInt16(t); }
inline int32_t network_to_host(const int32_t& t) { return OSSwapBigToHostInt32(t); }
inline uint32_t network_to_host(const uint32_t& t) { return OSSwapBigToHostInt32(t); }
inline int64_t network_to_host(const int64_t& t) { return OSSwapBigToHostInt64(t); }
inline uint64_t network_to_host(const uint64_t& t) { return OSSwapBigToHostInt64(t); }

// Convert from host to network byte order.  Overloaded for the integral types int{16,32,64}_t and
// uint{16,32,64}_t.
inline int16_t host_to_network(const int16_t& t) { return OSSwapHostToBigInt16(t); }
inline uint16_t host_to_network(const uint16_t& t) { return OSSwapHostToBigInt16(t); }
inline int32_t host_to_network(const int32_t& t) { return OSSwapHostToBigInt32(t); }
inline uint32_t host_to_network(const uint32_t& t) { return OSSwapHostToBigInt32(t); }
inline int64_t host_to_network(const int64_t& t) { return OSSwapHostToBigInt64(t); }
inline uint64_t host_to_network(const uint64_t& t) { return OSSwapHostToBigInt64(t); }

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
const uint8_t* NetworkBytes<T>::data() const {
    return _data;
}

template <typename T>
uint8_t* NetworkBytes<T>::mutable_data() {
    return _data;
}

template <typename T>
T NetworkBytes<T>::value() const {
    return network_to_host(*reinterpret_cast<const T*>(_data));
}

template <typename T>
void NetworkBytes<T>::set_value(T t) {
    *reinterpret_cast<T*>(_data) = host_to_network(t);
}

// Explicit template instantiation.
template class NetworkBytes<int16_t>;
template class NetworkBytes<uint16_t>;
template class NetworkBytes<int32_t>;
template class NetworkBytes<uint32_t>;
template class NetworkBytes<int64_t>;
template class NetworkBytes<uint64_t>;

}  // namespace sfz
