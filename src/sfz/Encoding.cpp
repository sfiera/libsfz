// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/Encoding.hpp"

#include <algorithm>
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Format.hpp"
#include "sfz/Macros.hpp"
#include "sfz/Range.hpp"
#include "sfz/String.hpp"

using std::min;

namespace sfz {

namespace {

// Identifies the code plane of the code point `code`.
//      
// Unicode partitions code points into 16 contiguous ranges of 65536 code points, known as planes.
// Plane 0, the basic multilingual plane, contains code points that are likely to be used.  Planes
// 1-16 are also defined, but anything else should be identified as invalid.
//
// @param [in] code     A code point to test.
// @returns             The code plane corresponding to `code`.
inline int unicode_plane(uint32_t code) {
    return code >> 16;
}   
    
// Identifies surrogate code points.
//
// UTF-16 represents code points outside the basic multilingual plane (plane 0) with a pair of
// plane 0 code points, the first of which is in the range U+D800 to U+DBFF, and the second of which
// is in the range U+DC00 to U+DFFF.  These are not valid code points within a String, so we need
// to identify them as invalid.
//  
// @param [in] code     A code point to test.
// @returns             true iff `code` is a surrogate code.
inline bool is_surrogate(uint32_t code) {
    return (unicode_plane(code) == 0) && ((code & 0xD800) == 0xD800);
}   

}  // namespace

const uint32_t kUnknownCodePoint = 0x00fffd;  // REPLACEMENT CHARACTER.
const uint32_t kAsciiUnknownCodePoint = 0x00003f;  // QUESTION MARK.

bool is_valid_code_point(uint32_t code) {
    return (unicode_plane(code) <= 16) && (!is_surrogate(code));
}   

Encoding::~Encoding() { }

bool Encoding::can_decode(const BytesPiece& bytes) const {
    uint32_t code;
    for (BytesPiece::const_iterator i = begin(bytes); i != end(bytes); next(bytes, &i)) {
        if (!dereference(bytes, i, &code)) {
            return false;
        }
    }
    return true;
}

void Encoding::decode(const BytesPiece& in, String* out) const {
    for (BytesPiece::const_iterator i = begin(in); i != end(in); next(in, &i)) {
        uint32_t code;
        if (dereference(in, i, &code)) {
            out->append(1, code);
        } else {
            out->append(1, kUnknownCodePoint);
        }
    }
}

bool Encoding::at(const BytesPiece& bytes, size_t loc, uint32_t* code) const {
    for (BytesPiece::const_iterator i = begin(bytes); i != end(bytes); next(bytes, &i)) {
        if (loc == 0) {
            return dereference(bytes, i, code);
        }
        --loc;
    }
    return false;
}

bool Encoding::empty(const BytesPiece& bytes) const {
    return begin(bytes) == end(bytes);
}

size_t Encoding::size(const BytesPiece& bytes) const {
    size_t size = 0;
    for (BytesPiece::const_iterator i = begin(bytes); i != end(bytes); next(bytes, &i)) {
        ++size;
    }
    return size;
}

StringPiece Encoding::substr(const BytesPiece& bytes, size_t loc) const {
    for (BytesPiece::const_iterator i = begin(bytes); i != end(bytes); next(bytes, &i)) {
        if (loc == 0) {
            return StringPiece(BytesPiece(i, bytes.end()), *this);
        } else {
            --loc;
        }
    }
    throw Exception(
            "Encoding::substr(x, {0}) called, but Encoding::size(x) == {1}",
            loc, this->size(bytes));
}

StringPiece Encoding::substr(const BytesPiece& bytes, size_t loc, size_t size) const {
    BytesPiece::const_iterator start = begin(bytes);
    for ( ; start != end(bytes); next(bytes, &start)) {
        if (loc == 0) {
            break;
        } else {
            --loc;
        }
    }
    for (BytesPiece::const_iterator i = start; i != end(bytes); next(bytes, &i)) {
        if (size == 0) {
            return StringPiece(BytesPiece(start, i), *this);
        } else {
            --size;
        }
    }
    throw Exception(
            "Encoding::substr(x, {0}, {1}) called, but Encoding::size(x) == {2}",
            loc, size, this->size(bytes));
}

bool Encoding::can_encode(const StringPiece& string) const {
    foreach (it, string) {
        if (!can_encode(*it)) {
            return false;
        }
    }
    return true;
}

void Encoding::encode(const StringPiece& in, Bytes* out) const {
    foreach (it, in) {
        uint32_t code = *it;
        if (can_encode(code)) {
            encode(code, out);
        } else if (can_encode(kUnknownCodePoint)) {
            encode(kUnknownCodePoint, out);
        } else {
            encode(kAsciiUnknownCodePoint, out);
        }
    }
}

namespace {

class AsciiEncoding : public Encoding {
  public:
    AsciiEncoding() { }

    virtual StringPiece name() const {
        return StringPiece("ASCII", ascii_encoding());
    }

    virtual bool can_decode(const BytesPiece& bytes) const {
        foreach (it, bytes) {
            if (static_cast<uint8_t>(*it) & 0x80) {
                return false;
            }
        }
        return true;
    }

    virtual void decode(const BytesPiece& in, String* out) const {
        foreach (it, in) {
            uint8_t code = *it;
            if (code & 0x80) {
                out->append(1, kUnknownCodePoint);
            } else {
                out->append(1, code);
            }
        }
    }

    virtual bool at(const BytesPiece& bytes, size_t loc, uint32_t* code) const {
        *code = static_cast<uint8_t>(bytes.at(loc));
        return (*code & 0x80) == 0;
    }

    virtual bool empty(const BytesPiece& bytes) const {
        return bytes.empty();
    }

    virtual size_t size(const BytesPiece& bytes) const {
        return bytes.size();
    }

    virtual StringPiece substr(const BytesPiece& bytes, size_t loc) const {
        return StringPiece(bytes.substr(loc), *this);
    }

    virtual StringPiece substr(const BytesPiece& bytes, size_t loc, size_t size) const {
        return StringPiece(bytes.substr(loc, size), *this);
    }

    virtual BytesPiece::const_iterator begin(const BytesPiece& bytes) const {
        return bytes.begin();
    }

    virtual BytesPiece::const_iterator end(const BytesPiece& bytes) const {
        return bytes.end();
    }

    virtual void next(const BytesPiece& bytes, BytesPiece::const_iterator* it) const {
        static_cast<void>(bytes);
        ++(*it);
    }

    virtual bool dereference(
            const BytesPiece& bytes, BytesPiece::const_iterator it, uint32_t* code) const {
        *code = static_cast<uint8_t>(*it);
        return (*code & 0x80) == 0;
    }

    virtual bool can_encode(uint32_t code) const {
        return code < 0x80;
    }

    virtual void encode(const StringPiece& in, Bytes* out) const {
        foreach (it, in) {
            encode(*it, out);
        }
    }

    virtual void encode(uint32_t code, Bytes* out) const {
        if (can_encode(code)) {
            out->append(1, code);
        } else {
            out->append(1, kAsciiUnknownCodePoint);
        }
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(AsciiEncoding);
};

}  // namespace

const Encoding& ascii_encoding() {
    static const AsciiEncoding instance;
    return instance;
}

namespace {

class Latin1Encoding : public Encoding {
  public:
    Latin1Encoding() { }

    virtual StringPiece name() const {
        return StringPiece("ISO-8859-1", ascii_encoding());
    }

    virtual bool can_decode(const BytesPiece& bytes) const {
        return true;
    }

    virtual void decode(const BytesPiece& in, String* out) const {
        foreach (it, in) {
            out->append(1, static_cast<uint8_t>(*it));
        }
    }

    virtual bool at(const BytesPiece& bytes, size_t loc, uint32_t* code) const {
        *code = static_cast<uint8_t>(bytes.at(loc));
        return true;
    }

    virtual bool empty(const BytesPiece& bytes) const {
        return bytes.empty();
    }

    virtual size_t size(const BytesPiece& bytes) const {
        return bytes.size();
    }

    virtual StringPiece substr(const BytesPiece& bytes, size_t loc) const {
        return StringPiece(bytes.substr(loc), *this);
    }

    virtual StringPiece substr(const BytesPiece& bytes, size_t loc, size_t size) const {
        return StringPiece(bytes.substr(loc, size), *this);
    }

    virtual BytesPiece::const_iterator begin(const BytesPiece& bytes) const {
        static_cast<void>(bytes);
        return bytes.begin();
    }

    virtual BytesPiece::const_iterator end(const BytesPiece& bytes) const {
        return bytes.end();
    }

    virtual void next(const BytesPiece& bytes, BytesPiece::const_iterator* it) const {
        ++(*it);
    }

    virtual bool dereference(
            const BytesPiece& bytes, BytesPiece::const_iterator it, uint32_t* code) const {
        *code = static_cast<uint8_t>(*it);
        return true;
    }

    virtual bool can_encode(uint32_t code) const {
        return code < 0x100;
    }

    virtual void encode(const StringPiece& in, Bytes* out) const {
        foreach (it, in) {
            encode(*it, out);
        }
    }

    virtual void encode(uint32_t code, Bytes* out) const {
        if (can_encode(code)) {
            out->append(1, code);
        } else {
            out->append(1, kAsciiUnknownCodePoint);
        }
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(Latin1Encoding);
};

}  // namespace

const Encoding& latin1_encoding() {
    static const Latin1Encoding instance;
    return instance;
}

namespace {

class Utf8Encoding : public Encoding {
  public:
    Utf8Encoding() { }

    virtual StringPiece name() const {
        return StringPiece("UTF-8", ascii_encoding());
    }

    virtual bool empty(const BytesPiece& bytes) const {
        return bytes.empty();
    }

    virtual BytesPiece::const_iterator begin(const BytesPiece& bytes) const {
        return bytes.begin();
    }

    virtual BytesPiece::const_iterator end(const BytesPiece& bytes) const {
        return bytes.end();
    }

    virtual void next(const BytesPiece& bytes, BytesPiece::const_iterator* it) const {
        uint32_t code;
        if (dereference(bytes, *it, &code)) {
            if (code < 0x80) {
                *it += 1;
            } else if (code < 0x800) {
                *it += 2;
            } else if (code < 0x10000) {
                *it += 3;
            } else {
                *it += 4;
            }
        } else {
            ++(*it);
        }
    }

    virtual bool dereference(
            const BytesPiece& bytes, BytesPiece::const_iterator it, uint32_t* code) const {
        *code = static_cast<uint8_t>(*it);
        if ((*code & 0x80) == 0) {
            return true;
        }
        size_t continuations;
        uint32_t min_code;
        if (!start_continuation(code, &continuations, &min_code)) {
            return false;
        }
        foreach (i, range(continuations)) {
            if (it + i + 1 == bytes.end()) {
                return false;
            }
            if (!continue_continuation(code, it[i + 1])) {
                return false;
            }
        }
        return (*code >= min_code) && is_valid_code_point(*code);
    }

    virtual bool can_encode(uint32_t code) const {
        return true;
    }

    virtual bool can_encode(const StringPiece& string) const {
        return true;
    }

    virtual void encode(uint32_t code, Bytes* out) const {
        if (code <= 0x7f) {
            out->append(1, code);
        } else if (code <= 0x7ff) {
            out->append(1, 0xc0 | (code >> 6));
            out->append(1, 0x80 | ((code >> 0) & 0x3f));
        } else if (code <= 0xffff) {
            out->append(1, 0xe0 | (code >> 12));
            out->append(1, 0x80 | ((code >> 6) & 0x3f));
            out->append(1, 0x80 | ((code >> 0) & 0x3f));
        } else {
            out->append(1, 0xf0 | (code >> 18));
            out->append(1, 0x80 | ((code >> 12) & 0x3f));
            out->append(1, 0x80 | ((code >> 6) & 0x3f));
            out->append(1, 0x80 | ((code >> 0) & 0x3f));
        }
    }

  private:
    // Attempts to start a multi-byte UTF-8 code point.
    //
    // @param [in, out] code The first byte of the potential multi-byte code point.  If it points
    //                      to a valid continuation starting code point, then the continuation bits
    //                      are removed, leaving only the content bits.
    // @param [out] continuations The number of continuation bytes expected after this one.
    // @param [out] min_code The minimum code point which can be represented by a multi-byte
    //                      code point of this size.
    // @returns             true iff `code` correctly started a multi-byte code point.
    bool start_continuation(uint32_t* code, size_t* continuations, uint32_t* min_code) const {
        if ((*code & 0xe0) == 0xc0) {
            *continuations = 1;
            *min_code = 0x80;
            *code &= 0x1f;
        } else if ((*code & 0xf0) == 0xe0) {
            *continuations = 2;
            *min_code = 0x800;
            *code &= 0x0f;
        } else if ((*code & 0xf1) == 0xf0) {
            *continuations = 3;
            *min_code = 0x10000;
            *code &= 0x0e;
        } else {
            return false;
        }
        return true;
    }

    // Attempts to continue a multi-byte UTF-8 code point.
    //
    // @param [in, out] code Updated with the new value of the code point.
    // @param [in] byte     The next byte in a multi-byte UTF-8 code point.
    // @returns             true iff `byte` correctly continued a multi-byte code point.
    bool continue_continuation(uint32_t* code, uint8_t byte) const {
        if ((byte & 0xc0) != 0x80) {
            return false;
        }
        *code <<= 6;
        *code |= byte & 0x3f;
        return true;
    }

    DISALLOW_COPY_AND_ASSIGN(Utf8Encoding);
};

}  // namespace

const Encoding& utf8_encoding() {
    static const Utf8Encoding instance;
    return instance;
}

}  // namespace sfz
