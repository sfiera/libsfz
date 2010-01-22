// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_ENCODING_HPP_
#define SFZ_ENCODING_HPP_

#include <stdint.h>
#include <stdlib.h>
#include "sfz/Bytes.hpp"
#include "sfz/Macros.hpp"
#include "sfz/SmartPtr.hpp"

namespace sfz {

class Bytes;
class BytesPiece;
class Encoding;
class String;
class StringPiece;

// ASCII text encoding.
//
// This encoding can represent code points in the range [U+00, U+7F].  It does so by representing
// each code point as a 1-byte value with the corresponding integer; the most-significant bit of
// the binary representation is never set.
//
// A StringPiece in ASCII encoding makes the following time guarantees:
//
//   construction: linear time.
//   size(): constant time.
//   at(): constant time.
//   empty(): constant time.
//   substr(): constant time.
//   iteration: linear time.
//
// @returns             An Encoding object for ASCII.
const Encoding& ascii_encoding();

// Latin-1 text encoding.
//
// This encoding can represent code points in the range [U+00, U+FF].  It does so by representing
// each code point as a 1-byte value with the corresponding integer.
//
// All ASCII code points are encoded equivalently in Latin-1; as a consequence, all valid
// ASCII-encoded strings are also Latin-1-encoded strings with equal values.  Code points in the
// Latin-1 supplement make up the other half of valid values.  All byte sequences are valid
// Latin-1-encoded strings.
//
// A StringPiece in Latin-1 encoding makes the following time guarantees:
//
//   construction: linear time.
//   size(): constant time.
//   at(): constant time.
//   empty(): constant time.
//   substr(): constant time.
//   iteration: linear time.
//
// @returns             An Encoding object for Latin-1.
const Encoding& latin1_encoding();

// UTF-8 text encoding.
//
// This encoding can represent any valid code point.  It is a variable-width encoding, taking up
// between 1 and 4 bytes for every code point.  As such, random access to UTF-8-encoded code points
// is linear time, making it a relatively inefficient in-memory format for strings.
//
// All ASCII code points are encoded equivalently in UTF-8; as a consequence, all valid
// ASCII-encoded strings are also UTF-8-encoded strings with equal values.  Other code points in
// the basic multilingual plane are encoded with 2 or 3 bytes, and code poins outside of it are
// encoded with 4 bytes.
//
// A StringPiece in UTF-8 encoding makes the following time guarantees:
//
//   construction: linear time.
//   iteration: linear time.
//   size(): linear time.
//   at(): linear time.
//   empty(): constant time.
//   substr(): linear time.
//
// @returns             An Encoding object for UTF-8.
const Encoding& utf8_encoding();

// Constants for replacing code points which cannot be encoded.
//
// Encodings are required to be able to encode any string to a byte sequence, and to decode any
// byte sequence into a string.  However, in a given encoding, a string may not be encodable, or a
// byte sequence not decodable.  In such cases, these code points are provided as replacements.
//
// If decoding a byte sequence to a string, or encoding a string into a byte sequence using an
// encoding that is capable of representing it, such as UTF-8 or UTF-32, then kUnknownCodePoint
// should be used.  If encoding a string into a byte sequence using an encoding that cannot
// represent kUnknownCodePoint, such as ASCII or Latin-1, then kAsciiUnknownCodePoint should be
// used instead.
extern const uint32_t kUnknownCodePoint;
extern const uint32_t kAsciiUnknownCodePoint;

// Identifies valid code points.
//
// Unicode specifies that only code points in the range [U+000000, U+10FFFF] are valid.  In
// addition, it guarantees that surrogate code points, which exist in the range [U+D800, U+DFFF],
// will never be assigned values.  This method returns false for any value of `code` which is
// outside the former, or inside the latter.
//
// @param [in] code     A code point to test.
// @returns             true iff `code` is valid.
bool is_valid_code_point(uint32_t code);

// A mapping between sequences of code points and sequences of bytes.
class Encoding {
  public:
    virtual ~Encoding();

    // @returns             The name of the encoding, in ASCII encoding.
    virtual StringPiece name() const = 0;

    // Determines whether `bytes` represents well-formed encoded data.
    //
    // The default implementation of this method loops over the string using the iterator methods.
    // If any code point could not be successfully dereferenced, then it returns false; otherwise,
    // it returns true.  Time complexity is linear in the size of `bytes`.
    //
    // @param [in] bytes    A sequence of arbitrary bytes.
    // @returns             true iff `bytes` can be decoded.
    virtual bool can_decode(const BytesPiece& bytes) const;

    // Appends the decoded contents of `in` to `out`.
    //
    // The default implementation of this method loops over the string using the iterator methods.
    // Any code point that could be successfully dereferenced is appended to `out`, with
    // kUnknownCodePoint replacing any code point that could not.  Time complexity is linear in the
    // size of `in`.
    //
    // @param [in] in       The byte sequence to decode from.
    // @param [out] out     The code point sequence to append to.
    virtual void decode(const BytesPiece& in, String* out) const;

    // Retrieves the `loc`th decoded code point in the represented string.
    //
    // The default implementation of this method loops over the string using the iterator methods.
    // If, on the `loc`th iteration, it successfully dereferences a code point, then it sets *code
    // and returns true; otherwise it returns false.
    //
    // Time complexity of the default implementation is linear in the size of `bytes`.  Any
    // encoding which can provide a constant time implementation of this method (e.g., any
    // fixed-width encoding) should override the default implementation.
    //
    // @param [in] bytes    The byte sequence to decode from.
    // @param [in] loc      An index into the code point sequence represented by `bytes`.
    // @param [out] code    Set to the `loc`th decoded point if decoding succeeded.  On failure,
    //                      the previous value may be overwritten, and should not be used.
    // @returns             true iff the `loc`th code point could be decoded.
    virtual bool at(const BytesPiece& bytes, size_t loc, uint32_t* code) const;

    // Determines if there are any code points in the represented string.
    //
    // The default implementation of this method compares the results of the begin() and end()
    // iterators, and returns true if they are equal.
    //
    // @param [in] bytes    The byte sequence to decode from.
    // @returns             true iff `bytes` is empty.
    virtual bool empty(const BytesPiece& bytes) const;

    // Determines the number of code points in the represented string.
    //
    // The default implementation of this method loops over the string using the iterator methods.
    // The total number of iterations is returned.
    //
    // Time complexity of the default implementation is linear in the size of `bytes`.  Any
    // encoding which can provide a constant time implementation of this method (e.g., any
    // fixed-width encoding) should override the default implementation.
    //
    // @param [in] bytes    The byte sequence to decode from.
    // @returns             The number of code points in the string.
    virtual size_t size(const BytesPiece& bytes) const;

    // Returns a substring of the represented string.
    //
    // If the requested substring is not valid, then an exception is thrown.
    //
    // The default implementation of this method loops over the string using the iterator methods.
    // A substring of `bytes` is returned, from the iterator position during the `loc`th iteration
    // to the (`loc` + `size`)th iteration, using *this as the encoding.
    //
    // Time complexity of the default implementation is linear in the size of `bytes`.  Any
    // encoding which can provide a constant time implementation of this method (e.g., any
    // fixed-width encoding) should override the default implementation.
    //
    // @param [in] bytes    The byte sequence to decode from.
    // @param [in] loc      The position of the first code point to return.
    // @param [in] size     The number of code points in the returned substring.
    // @returns             The requested substring, presumably taken from `bytes`.
    // @throws Exception    If loc or size exceeds the size of the represented string.
    virtual StringPiece substr(const BytesPiece& bytes, size_t loc) const;
    virtual StringPiece substr(const BytesPiece& bytes, size_t loc, size_t size) const;

    // Returns iterators to the beginning and end of the represented string.
    //
    // begin() must return the iterator within `bytes` to the start of the first valid code point.
    // end() must return the result of advancing an iterator from the start of last valid code
    // point in `bytes`.
    //
    // In general, constant-time implementations of begin() and end() are expected.  Linear- or
    // sub-linear-time is required, in order for the default implementations of the above methods
    // to make their complexity guarantees.
    //
    // @param [in] bytes    The byte sequence to decode from.
    // @returns             An iterator to the beginning or end of the represented string.
    virtual BytesPiece::const_iterator begin(const BytesPiece& bytes) const = 0;
    virtual BytesPiece::const_iterator end(const BytesPiece& bytes) const = 0;

    // Advances `*it` to the start of the next code point in the represented string.
    //
    // If a valid code point represented by N bytes starts at `*it` in `bytes`, then `*it` should
    // be advanced N times.  Otherwise, it should be advanced once.
    //
    // Constant-time implementation of next() is required in order for the default implementations
    // of the above methods to make their time complexity guarantees.
    //
    // @param [in] bytes    The byte sequence to decode from.
    // @param [in, out] it  An iterator at the start of a code point in the byte sequence.  After
    //                      this method call, is set to the iterator of the code point following
    //                      the one it initially pointed to.
    virtual void next(const BytesPiece& bytes, BytesPiece::const_iterator* it) const = 0;

    // Retrieves the code point starting at position `it` within `bytes`.
    //
    // If `it` points to the start of a valid code point in `bytes`, then `*code` should be set to
    // the value of that code point, and true returned.  Otherwise, false should be returned.
    //
    // Constant-time implementation of next() is required in order for the default implementations
    // of the above methods to make their time complexity guarantees.
    //
    // @param [in] bytes    The byte sequence to decode from.
    // @param [in] it       An iterator over the byte sequence.
    // @param [out] code    Set to the decoded code point if decoding succeeded.  On failure, the
    //                      previous value may be overwritten, and should not be used.
    // @returns             true iff a code point could be decoded at `it`.
    virtual bool dereference(
            const BytesPiece& bytes, BytesPiece::const_iterator it, uint32_t* code) const = 0;

    // Identifies whether `string` contains data which can be encoded in this encoding.
    //
    // The default implementation of this method loops over the string using the iterator methods.
    // If can_encode(code) returns true for every code point `code` in the string, returns true;
    // otherwise, returns false.
    //
    // Time complexity of the default implementation is linear in the size of `string`.  Any
    // encoding which is capable of encoding any valid code point (e.g., UTF-8, UTF-16) should
    // override the default implementation to return true in constant time.
    //
    // @param [in] string   A sequence of code points.
    // @returns             true iff `string` can be encoded.
    virtual bool can_encode(const StringPiece& string) const;

    // Identifies whether `code` can be encoded in this encoding.
    //
    // Constant-time implementation of can_encode() is required in order for the default
    // implementations of encode() and can_encode() to make their time complexity guarantees.
    //
    // @returns             true iff `code` can be encoded.
    virtual bool can_encode(uint32_t code) const = 0;

    // Appends the decoded contents of `in` to `out`.
    //
    // The default implementation of this method loops over the string using the iterator methods.
    // Any code point that can be encoded is appended to `out` using encode().  For each code point
    // that could not be encoded, kUnknownCodePoint is encoded, or, if that could not be encoded
    // either, kAsciiUnknownCodePoint.  Time complexity is linear in the size of `in`.
    //
    // @param [in] in       The code point sequence to encode from.
    // @param [out] out     The byte sequence to append to.
    virtual void encode(const StringPiece& in, Bytes* out) const;

    // Appends the result of encoding `code` to `out`.
    //
    // Constant-time implementation of can_encode() is required in order for the default
    // implementations of encode() and can_encode() to make their time complexity guarantees.
    //
    // @param [in] code     The code point to encode.
    // @param [out] out     The byte sequence to append to.
    virtual void encode(uint32_t code, Bytes* out) const = 0;
};

}  // namespace sfz

#endif  // SFZ_ENCODING_HPP_
