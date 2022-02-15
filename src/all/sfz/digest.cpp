// Copyright (c) 2010-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/digest.hpp>

#include <string.h>
#include <limits>
#include <pn/input>
#include <sfz/encoding.hpp>
#include <sfz/file.hpp>
#include <sfz/os.hpp>
#include <stdexcept>

using std::numeric_limits;

namespace sfz {

sha1::sha1() { reset(); }

sha1::sha1(const sha1& other) { memcpy(this, &other, sizeof(sha1)); }

void sha1::reset() {
    _size                = 0;
    _message_block_index = 0;
    _intermediate.d[0]   = 0x67452301;
    _intermediate.d[1]   = 0xefcdab89;
    _intermediate.d[2]   = 0x98badcfe;
    _intermediate.d[3]   = 0x10325476;
    _intermediate.d[4]   = 0xc3d2e1f0;
}

void sha1::write(pn::data_view input) {
    if (input.empty()) {
        return;
    }
    if (((numeric_limits<uint64_t>::max() - _size) / 8) <
        static_cast<std::make_unsigned<pn::data_view::size_type>::type>(input.size())) {
        throw std::runtime_error("message is too long");
    }
    pn::data_view remainder = input;
    while (_message_block_index + remainder.size() >= 64) {
        const int size = 64 - _message_block_index;
        memcpy(_message_block + _message_block_index, remainder.data(), size);
        remainder.shift(size);
        process_message_block();
    }
    memcpy(_message_block + _message_block_index, remainder.data(), remainder.size());
    _message_block_index += remainder.size();
    _size += 8 * input.size();
}

sha1::digest sha1::compute() const {
    sha1 copy(*this);
    copy.finish();
    return copy._intermediate;
}

void sha1::finish() {
    _message_block[_message_block_index++] = 0x80;
    if (_message_block_index > 56) {
        memset(_message_block + _message_block_index, '\0', 64 - _message_block_index);
        process_message_block();
    }
    memset(_message_block + _message_block_index, '\0', 56 - _message_block_index);
    pn::data size_bytes;
    size_bytes.output().write(_size);
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

void sha1::process_message_block() {
    static const uint32_t k[] = {
            0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6,
    };

    uint32_t  w[80];
    pn::input block = pn::data_view{_message_block, 64}.input();
    for (int i = 0; i < 16; ++i) {
        block.read(&w[i]);
    }
    for (int i = 16; i < 80; ++i) {
        w[i] = left_rotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    uint32_t a = _intermediate.d[0];
    uint32_t b = _intermediate.d[1];
    uint32_t c = _intermediate.d[2];
    uint32_t d = _intermediate.d[3];
    uint32_t e = _intermediate.d[4];

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

    _intermediate.d[0] += a;
    _intermediate.d[1] += b;
    _intermediate.d[2] += c;
    _intermediate.d[3] += d;
    _intermediate.d[4] += e;
    _message_block_index = 0;
}

sha1::digest::digest(pn::data_view data) {
    if (data.size() != 20) {
        throw std::runtime_error(
                pn::format("sha1 digest created from data of size {0}", data.size()).c_str());
    }
    d[0] = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) |
           ((uint32_t)data[3]);
    d[1] = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) |
           ((uint32_t)data[7]);
    d[2] = ((uint32_t)data[8] << 24) | ((uint32_t)data[9] << 16) | ((uint32_t)data[10] << 8) |
           ((uint32_t)data[11]);
    d[3] = ((uint32_t)data[12] << 24) | ((uint32_t)data[13] << 16) | ((uint32_t)data[14] << 8) |
           ((uint32_t)data[15]);
    d[4] = ((uint32_t)data[16] << 24) | ((uint32_t)data[17] << 16) | ((uint32_t)data[18] << 8) |
           ((uint32_t)data[19]);
}

pn::data sha1::digest::data() const {
    pn::data   out;
    pn::output f = out.output();
    for (int i = 0; i < 5; ++i) {
        f.write(d[i]);
    }
    return out;
}

pn::string sha1::digest::hex() const {
    char  buf[41];
    char* ptr = buf;
    for (int i = 0; i < 5; ++i) {
        sprintf(ptr, "%08x", d[i]);
        ptr += 8;
    }
    return pn::string_view{buf, 40}.copy();
}

sha1::digest file_digest(pn::string_view path) {
    mapped_file file(path);
    sha1        sha;
    sha.write(file.data());
    return sha.compute();
}

sha1::digest tree_digest(pn::string_view path) {
    if (!path::isdir(path)) {
        return file_digest(path);
    }
    struct digestWalker : TreeWalker {
        // For files, hash the size and bytes of their UTF-8-encoded path, followed by the size and
        // bytes of the file content.  We don't worry about the mode or owner of the file, just as
        // we wouldn't if taking the digest of a file.
        void file(pn::string_view path, const Stat&) const {
            pn::data_view path_bytes{reinterpret_cast<const uint8_t*>(path.data() + prefix_size),
                                     path.size() - prefix_size};
            sha.write<uint64_t>(path_bytes.size());
            sha.write(path_bytes);

            mapped_file file(path);
            sha.write<uint64_t>(file.data().size());
            sha.write(file.data());
        }

        // Ignore empty directories.  Directories which are not empty will be included in the
        // resulting digest by virtue of the inclusion of their files.
        void pre_directory(pn::string_view path, const Stat& stat) const {
            static_cast<void>(path);
            static_cast<void>(stat);
        }
        void post_directory(pn::string_view path, const Stat& stat) const {
            static_cast<void>(path);
            static_cast<void>(stat);
        }

        // Throw exceptions on anything that might break our logical view of a tree as a
        // hierarchical listing of of regular files.
        void cycle_directory(pn::string_view path, const Stat&) const {
            throw std::runtime_error(pn::format("Found directory cycle: {0}.", path).c_str());
        }
        void other(pn::string_view path, const Stat&) const {
            throw std::runtime_error(pn::format("Found non-regular file: {0}", path).c_str());
        }

        // Ignore broken symlinks; they effectively don't exist.
        void broken_symlink(pn::string_view path, const Stat& stat) const {
            static_cast<void>(path);
            static_cast<void>(stat);
        }

        // Can't happen during WALK_LOGICAL.
        void symlink(pn::string_view path, const Stat& stat) const {
            static_cast<void>(path);
            static_cast<void>(stat);
        }

        sha1&     sha;
        const int prefix_size;
        digestWalker(sha1& sha, int prefix_size) : sha(sha), prefix_size(prefix_size) {}
    };
    sha1 sha;
    walk(path, WALK_LOGICAL, digestWalker(sha, path.size() + 1));
    return sha.compute();
}

bool operator==(const sha1::digest& lhs, const sha1::digest& rhs) {
    return memcmp(lhs.d, rhs.d, 5 * sizeof(uint32_t)) == 0;
}

bool operator!=(const sha1::digest& lhs, const sha1::digest& rhs) { return !(lhs == rhs); }

}  // namespace sfz
