// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_DIGEST_HPP_
#define SFZ_DIGEST_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <pn/data>
#include <pn/file>
#include <pn/string>

namespace sfz {

// Computes the SHA-1 digest of some sequence of bytes.
//
// Based on the code provided by RFC 3174.
class sha1 {
  public:
    struct digest {
        constexpr digest() : d{} {}
        constexpr digest(uint32_t d0, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4)
                : d{d0, d1, d2, d3, d4} {}
        digest(pn::data_view data);

        pn::data   data() const;
        pn::string hex() const;

        uint32_t d[5];
    };

    // Creates an instance in initial state.  Calling get_digest() on a default-initialized object
    // will result in the digest of 0 bytes of data.
    sha1();

    // Copies state derived from previous calls to `other.update()`.  This can be used to
    // efficiently compute the hash of several pieces of data which share a large, common prefix.
    // @param [in] other    The instance to copy from.
    explicit sha1(const sha1& other);

    // Resets the object to its initial state, as if update() had never been called.
    void reset();

    // Adds data in `input` to the current content.  It is more efficient, though semantically
    // identical, to add data in larger chunks.
    // @param [in] input    The data to add to the digest.
    void write(pn::data_view input);
    template <typename... arguments>
    void write(const arguments&... args) {
        pn::data d;
        {
            pn::file f = d.open("w");
            f.write(args...);
        }
        write(pn::data_view{d});
    }

    // Returns a digest computed from the current content.  This method does non-trivial work, so
    // if the digest is to be used multiple times, it should be called once, and the retrieved
    // digest reused.
    digest compute() const;

  private:
    // Finishes computation of the digest of the current contents.  After this method is called, it
    // is no longer valid to call update().  The implementation of digest() therefore copies *this
    // and calls finish() on the copy, rather than changing *this.
    //
    // After calling this method, _intermediate will contain the final hashed value of the content.
    void finish();

    // Processes the content in _message_block and resets _message_block_index to 0.  This should
    // be called only when all 64 bytes of data in _message_block have been filled with content.
    void process_message_block();

    // The current value of the digest being computed.
    struct digest _intermediate;

    // The size of the current content.
    uint64_t _size;

    // Used to accumulate data before a call to process_message_block().  The next byte of data
    // will be added at _message_block_index, and when _message_block_index is 64, at the end of
    // _message_block, process_message_block() is called to process the data and reset it to 0.
    int     _message_block_index;
    uint8_t _message_block[64];

    // Disallow assignment.  Copying is allowed but explicit.
    sha1& operator=(const sha1&);
};

bool operator==(const sha1::digest& lhs, const sha1::digest& rhs);
bool operator!=(const sha1::digest& lhs, const sha1::digest& rhs);

// Hashes a regular file.
sha1::digest file_digest(pn::string_view path);

// Hashes a tree containing regular files (or symlinks).
sha1::digest tree_digest(pn::string_view path);

}  // namespace sfz

#endif  // SFZ_DIGEST_HPP_
