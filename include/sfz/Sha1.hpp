// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_SHA1_HPP_
#define SFZ_SHA1_HPP_

#include <stdint.h>
#include <stdlib.h>

namespace sfz {

class Bytes;
class BytesPiece;

// Computes the SHA-1 digest of some sequence of bytes.
//
// Based on the code provided by RFC 3174.
class Sha1 {
  public:
    // Creates an instance in initial state.  Calling get_digest() on a default-initialized object
    // will result in the digest of 0 bytes of data.
    Sha1();

    // Copies state derived from previous calls to `other.update()`.  This can be used to
    // efficiently compute the hash of several pieces of data which share a large, common prefix.
    // @param [in] other    The instance to copy from.
    explicit Sha1(const Sha1& other);

    // Resets the object to its initial state, as if update() had never been called.
    void reset();

    // Adds data in `input` to the current content.  It is more efficient, though semantically
    // identical, to add data in larger chunks.
    // @param [in] input    The data to add to the digest.
    void append(const BytesPiece& input);
    void append(size_t num, uint8_t byte);

    // Appends the digest computed from the current content to `digest`.  This method does
    // non-trivial work, so if the digest is to be used multiple times, it should be called once,
    // and the retrieved digest reused.
    // @param [in] digest   The object to append the digest to.
    void get_digest(Bytes* digest) const;

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
    uint32_t _intermediate[5];

    // The size of the current content.
    uint64_t _size;

    // Used to accumulate data before a call to process_message_block().  The next byte of data
    // will be added at _message_block_index, and when _message_block_index is 64, at the end of
    // _message_block, process_message_block() is called to process the data and reset it to 0.
    int _message_block_index;
    uint8_t _message_block[64];

    // Disallow assignment.  Copying is allowed but explicit.
    Sha1& operator=(const Sha1&);
};

}  // namespace sfz

#endif  // SFZ_SHA1_HPP_
