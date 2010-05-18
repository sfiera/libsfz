// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/Sha1.hpp"

#include <limits>
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"
#include "sfz/NetworkBytes.hpp"
#include "sfz/ReadItem.hpp"
#include "sfz/WriteItem.hpp"

using std::numeric_limits;

namespace sfz {

Sha1::Sha1() {
    reset();
}

Sha1::Sha1(const Sha1& other) {
    memcpy(this, &other, sizeof(Sha1));
}

void Sha1::reset() {
    _size = 0;
    _message_block_index = 0;
    _intermediate.digest[0] = 0x67452301;
    _intermediate.digest[1] = 0xefcdab89;
    _intermediate.digest[2] = 0x98badcfe;
    _intermediate.digest[3] = 0x10325476;
    _intermediate.digest[4] = 0xc3d2e1f0;
}

void Sha1::append(const BytesPiece& input) {
    if (input.empty()) {
        return;
    }
    if ((numeric_limits<uint64_t>::max() - _size) < input.size()) {
        throw Exception("message is too long");
    }
    BytesPiece remainder = input;
    while (_message_block_index + remainder.size() >= 64) {
        const int size = 64 - _message_block_index;
        remainder.shift(_message_block + _message_block_index, size);
        process_message_block();
    }
    memcpy(_message_block + _message_block_index, remainder.data(), remainder.size());
    _message_block_index += remainder.size();
    _size += 8 * input.size();
}

void Sha1::append(size_t num, uint8_t byte) {
    Bytes bytes(num, byte);
    append(bytes);
}

Sha1::Digest Sha1::digest() const {
    Sha1 copy(*this);
    copy.finish();
    return copy._intermediate;
}

void Sha1::finish() {
    _message_block[_message_block_index++] = 0x80;
    if (_message_block_index > 56) {
        bzero(_message_block + _message_block_index, 64 - _message_block_index);
        process_message_block();
    }
    bzero(_message_block + _message_block_index, 56 - _message_block_index);
    NetworkBytes<uint64_t> size_bytes(_size);
    memcpy(_message_block + 56, size_bytes.data(), 8);
    process_message_block();
}

namespace {

// Does a circular rotation of `word`, shifting it `bits` bits to the left, and moving the bits
// that were shifted out to the right end of the word.
inline uint32_t left_rotate(uint32_t word, int bits) {
    return ((word << bits) | (word >> (32 - bits)));
}

// Does the permutation of values for a round of SHA-1.
inline void permute(
        uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d, uint32_t* e, uint32_t next) {
    *e = *d;
    *d = *c;
    *c = left_rotate(*b, 30);
    *b = *a;
    *a = left_rotate(*a, 5) + next;
}

}  // namespace

void Sha1::process_message_block() {
    static const uint32_t k[] = { 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6, };

    uint32_t w[80];
    BytesPiece block(_message_block, 64);
    read(&block, w, 16);
    for (int i = 16; i < 80; ++i) {
        w[i] = left_rotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    uint32_t a = _intermediate.digest[0];
    uint32_t b = _intermediate.digest[1];
    uint32_t c = _intermediate.digest[2];
    uint32_t d = _intermediate.digest[3];
    uint32_t e = _intermediate.digest[4];

    for (int i = 0; i < 20; ++i) {
        permute(&a, &b, &c, &d, &e, ((b & c) | ((~b) & d)) + e + w[i] + k[0]);
    }
    for (int i = 20; i < 40; ++i) {
        permute(&a, &b, &c, &d, &e, (b ^ c ^ d) + e + w[i] + k[1]);
    }
    for (int i = 40; i < 60; ++i) {
        permute(&a, &b, &c, &d, &e, ((b & c) | (b & d) | (c & d)) + e + w[i] + k[2]);
    }
    for (int i = 60; i < 80; ++i) {
        permute(&a, &b, &c, &d, &e, (b ^ c ^ d) + e + w[i] + k[3]);
    }

    _intermediate.digest[0] += a;
    _intermediate.digest[1] += b;
    _intermediate.digest[2] += c;
    _intermediate.digest[3] += d;
    _intermediate.digest[4] += e;
    _message_block_index = 0;
}

void write_to(WriteTarget out, const Sha1::Digest& digest) {
    write(out, digest.digest, 5);
}

}  // namespace sfz
