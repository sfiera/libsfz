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
#include "sfz/Rune.hpp"
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
extern const Rune kUnknownCodePoint;
extern const Rune kAsciiUnknownCodePoint;

// Identifies valid code points.
//
// Unicode specifies that only code points in the range [U+000000, U+10FFFF] are valid.  In
// addition, it guarantees that surrogate code points, which exist in the range [U+D800, U+DFFF],
// will never be assigned values.  This method returns false for any value of `rune` which is
// outside the former, or inside the latter.
//
// @param [in] rune     A potential code point to test.
// @returns             true iff `rune` is a valid code point.
bool is_valid_code_point(Rune rune);

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
    virtual bool can_decode(const BytesPiece& bytes) const = 0;

    // Appends the decoded contents of `in` to `out`.
    //
    // The default implementation of this method loops over the string using the iterator methods.
    // Any code point that could be successfully dereferenced is appended to `out`, with
    // kUnknownCodePoint replacing any code point that could not.  Time complexity is linear in the
    // size of `in`.
    //
    // @param [in] in       The byte sequence to decode from.
    // @param [out] out     The code point sequence to append to.
    virtual void decode(const BytesPiece& in, String* out) const = 0;

    // Identifies whether `string` contains data which can be encoded in this encoding.
    //
    // Time complexity is linear in the size of `string`.
    //
    // @param [in] string   A sequence of code points.
    // @returns             true iff `string` can be encoded.
    virtual bool can_encode(const StringPiece& string) const = 0;

    // Appends the decoded contents of `in` to `out`.
    //
    // Any code point that can be encoded is appended to `out`.  For each code point that could not
    // be encoded, kUnknownCodePoint is appended, or, if that could not be encoded either,
    // kAsciiUnknownCodePoint.  Time complexity is linear in the size of `in`.
    //
    // @param [in] in       The code point sequence to encode from.
    // @param [out] out     The byte sequence to append to.
    virtual void encode(const StringPiece& in, Bytes* out) const = 0;
};

}  // namespace sfz

#endif  // SFZ_ENCODING_HPP_
