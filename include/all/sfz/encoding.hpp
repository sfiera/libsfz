// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_ENCODING_HPP_
#define SFZ_ENCODING_HPP_

#include <stdint.h>
#include <pn/string>

namespace sfz {

// Constants for replacing code points which cannot be encoded.
//
// Encodings are required to be able to encode any string to a byte sequence, and to decode any
// byte sequence into a string.  However, in a given encoding, a string may not be encodable, or a
// byte sequence not decodable.  In such cases, these code points are provided as replacements.
//
// If decoding a byte sequence to a string, or encoding a string into a byte sequence using an
// encoding that is capable of representing it, such as UCS-2, then kUnknownCodePoint should be
// used.  If encoding a string into a byte sequence using an encoding that cannot represent
// kUnknownCodePoint, such as ASCII or Latin-1, then kAsciiUnknownCodePoint should be used instead.
extern const pn::rune kUnknownCodePoint;
extern const pn::rune kAsciiUnknownCodePoint;

// Identifies valid code points.
//
// Unicode specifies that only code points in the range [U+000000, U+10FFFF] are valid.  In
// addition, it guarantees that surrogate code points, which exist in the range [U+D800, U+DFFF],
// will never be assigned values.  This method returns false for any value of `rune` which is
// outside the former, or inside the latter.
//
// @param [in] rune     A potential code point to test.
// @returns             true iff `rune` is a valid code point.
bool is_valid_code_point(uint32_t rune);

// ASCII text encoding.
//
// This encoding can represent code points in the range [U+00, U+7F].  It does so by representing
// each code point as a 1-byte value with the corresponding integer; the most-significant bit of
// the binary representation is never set.
namespace ascii {

pn::data   encode(pn::string_view string);
pn::string decode(pn::data_view data);

}  // namespace ascii

// Latin-1 text encoding.
//
// This encoding can represent code points in the range [U+00, U+FF].  It does so by representing
// each code point as a 1-byte value with the corresponding integer.
//
// All ASCII code points are encoded equivalently in Latin-1; as a consequence, all valid
// ASCII-encoded strings are also Latin-1-encoded strings with equal values.  Code points in the
// Latin-1 supplement make up the other half of valid values.  All byte sequences are valid
// Latin-1-encoded strings.
namespace latin1 {

pn::data   encode(pn::string_view string);
pn::string decode(pn::data_view data);

}  // namespace latin1

// MacRoman text encoding.
//
// This encoding can represent code points in the range [U+00, U+7F], as well as a further set of
// 128 points.  It represents each code point as a 1-byte value with the corresponding integer.
//
// All ASCII code points are encoded equivalently in MacRoman; as a consequence, all valid
// ASCII-encoded strings are also MacRoman-encoded strings with equal values.  Code points in the
// MacRoman supplement make up the other half of valid values.  All byte sequences are valid
// MacRoman-encoded strings.
namespace macroman {

pn::data   encode(pn::string_view string);
pn::string decode(pn::data_view data);

}  // namespace macroman

}  // namespace sfz

#endif  // SFZ_ENCODING_HPP_
