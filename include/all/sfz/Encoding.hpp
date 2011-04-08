// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_ENCODING_HPP_
#define SFZ_ENCODING_HPP_

#include <sfz/print.hpp>
#include <sfz/write.hpp>

namespace sfz {

class BytesSlice;
class StringSlice;
template <typename T> struct EncodedString;
template <typename T> struct DecodedBytes;

template <typename T> struct EncodedString {
    const BytesSlice& bytes;
    EncodedString(const BytesSlice& b) : bytes(b) { }
};

template <typename T> struct DecodedBytes {
    const StringSlice& string;
    DecodedBytes(const StringSlice& s) : string(s) { }
};

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

// ASCII text encoding.
//
// This encoding can represent code points in the range [U+00, U+7F].  It does so by representing
// each code point as a 1-byte value with the corresponding integer; the most-significant bit of
// the binary representation is never set.
namespace ascii {

struct Ascii {
    static void encode_to(WriteTarget out, const StringSlice& string);
    static void decode_to(PrintTarget out, const BytesSlice& bytes);
};

inline DecodedBytes<Ascii> encode(const StringSlice& string) { return string; }
inline EncodedString<Ascii> decode(const BytesSlice& bytes) { return bytes; }

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

struct Latin1 {
    static void encode_to(WriteTarget out, const StringSlice& string);
    static void decode_to(PrintTarget out, const BytesSlice& bytes);
};

inline DecodedBytes<Latin1> encode(const StringSlice& string) { return string; }
inline EncodedString<Latin1> decode(const BytesSlice& bytes) { return bytes; }

}  // namespace latin1

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
namespace utf8 {

struct Utf8 {
    static void encode_to(WriteTarget out, const StringSlice& string);
    static void decode_to(PrintTarget out, const BytesSlice& bytes);
};

inline DecodedBytes<Utf8> encode(const StringSlice& string) { return string; }
inline EncodedString<Utf8> decode(const BytesSlice& bytes) { return bytes; }

}  // namespace latin1

template <typename T> void print_to(PrintTarget out, const EncodedString<T>& string) {
    T::decode_to(out, string.bytes);
}

template <typename T> void write_to(WriteTarget out, const DecodedBytes<T>& bytes) {
    T::encode_to(out, bytes.string);
}

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

struct MacRoman {
    static void encode_to(WriteTarget out, const StringSlice& string);
    static void decode_to(PrintTarget out, const BytesSlice& bytes);
};

inline DecodedBytes<MacRoman> encode(const StringSlice& string) { return string; }
inline EncodedString<MacRoman> decode(const BytesSlice& bytes) { return bytes; }

}  // namespace latin1

}  // namespace sfz

#endif  // SFZ_ENCODING_HPP_
