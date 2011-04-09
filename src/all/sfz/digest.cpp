// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/digest.hpp"

#include <limits>
#include "sfz/Exception.hpp"
#include "sfz/NetworkBytes.hpp"
#include "sfz/MappedFile.hpp"
#include "sfz/encoding.hpp"
#include "sfz/bytes.hpp"
#include "sfz/format.hpp"
#include "sfz/os.hpp"
#include "sfz/read.hpp"
#include "sfz/write.hpp"

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

void Sha1::push(const BytesSlice& input) {
    if (input.empty()) {
        return;
    }
    if (((numeric_limits<uint64_t>::max() - _size) / 8) < input.size()) {
        throw Exception("message is too long");
    }
    BytesSlice remainder = input;
    while (_message_block_index + remainder.size() >= 64) {
        const int size = 64 - _message_block_index;
        remainder.shift(_message_block + _message_block_index, size);
        process_message_block();
    }
    memcpy(_message_block + _message_block_index, remainder.data(), remainder.size());
    _message_block_index += remainder.size();
    _size += 8 * input.size();
}

void Sha1::push(size_t num, uint8_t byte) {
    Bytes bytes(num, byte);
    push(bytes);
}

Sha1::Digest Sha1::digest() const {
    Sha1 copy(*this);
    copy.finish();
    return copy._intermediate;
}

void Sha1::finish() {
    _message_block[_message_block_index++] = 0x80;
    if (_message_block_index > 56) {
        memset(_message_block + _message_block_index, '\0', 64 - _message_block_index);
        process_message_block();
    }
    memset(_message_block + _message_block_index, '\0', 56 - _message_block_index);
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
    BytesSlice block(_message_block, 64);
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

void read_from(ReadSource in, Sha1::Digest* digest) {
    read(in, digest->digest, 5);
}

void write_to(WriteTarget out, const Sha1::Digest& digest) {
    write(out, digest.digest, 5);
}

void print_to(PrintTarget out, const Sha1::Digest& digest) {
    for (int i = 0; i < 5; ++i) {
        print(out, hex(digest.digest[i], 8));
    }
}

Sha1::Digest file_digest(const StringSlice& path) {
    MappedFile file(path);
    Sha1 sha;
    sha.push(file.data());
    return sha.digest();
}

Sha1::Digest tree_digest(const StringSlice& path) {
    if (!path::isdir(path)) {
        return file_digest(path);
    }
    struct DigestWalker : TreeWalker {
        // For files, hash the size and bytes of their UTF-8-encoded path, followed by the size and
        // bytes of the file content.  We don't worry about the mode or owner of the file, just as
        // we wouldn't if taking the digest of a file.
        void file(const StringSlice& path, const Stat&) {
            Bytes path_bytes(utf8::encode(path.slice(prefix_size)));
            write<uint64_t>(&sha, path_bytes.size());
            sha.push(path_bytes);

            MappedFile file(path);
            write<uint64_t>(&sha, file.data().size());
            sha.push(file.data());
        }

        // Ignore empty directories.  Directories which are not empty will be included in the
        // resulting digest by virtue of the inclusion of their files.
        void pre_directory(const StringSlice& path, const Stat&) { }
        void post_directory(const StringSlice& path, const Stat&) { }

        // Throw exceptions on anything that might break our logical view of a tree as a
        // hierarchical listing of of regular files.
        void cycle_directory(const StringSlice& path, const Stat&) {
            throw Exception(format("Found directory cycle: {0}.", path));
        }
        void other(const StringSlice& path, const Stat&) {
            throw Exception(format("Found non-regular file: {0}", path));
        }

        // Ignore broken symlinks; they effectively don't exist.
        void broken_symlink(const StringSlice& path, const Stat&) { }

        // Can't happen during WALK_LOGICAL.
        void symlink(const StringSlice& path, const Stat&) { }

        Sha1 sha;

        const int prefix_size;
        DigestWalker(int prefix_size) : prefix_size(prefix_size) { }
    } walker(path.size() + 1);
    walk(path, WALK_LOGICAL, &walker);
    return walker.sha.digest();
}

bool operator==(const Sha1::Digest& lhs, const Sha1::Digest& rhs) {
    return memcmp(lhs.digest, rhs.digest, 5 * sizeof(uint32_t)) == 0;
}

bool operator!=(const Sha1::Digest& lhs, const Sha1::Digest& rhs) {
    return !(lhs == rhs);
}

}  // namespace sfz
