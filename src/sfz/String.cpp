// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include "sfz/String.hpp"

#include <string.h>
#include <algorithm>
#include "sfz/Bytes.hpp"
#include "sfz/Encoding.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"

using std::min;

namespace sfz {

// Internal encoding used by String.
//
// This Encoding encodes strings identically to native-endian UTF-32 with no byte-order marker.
// However, since it is used only internally, it is given special dispensation when decoding byte
// sequences to make extra assumptions about the content of those sequences.  Specifically, it
// never checks that the number of bits in the sequence is evenly divisible by 32, or that any
// individual 32-bit value is a valid code point.
//
// All of the virtual methods in Encoding are overridden, even when the defaults would do.  Because
// it is used in almost every method call on String, it is important that this Encoding be fast.
class String::InternalEncoding : public Encoding {
  public:
    InternalEncoding() { }

    virtual StringPiece name() const {
        return StringPiece("Internal-32", ascii_encoding());
    }

    virtual bool can_decode(const BytesPiece& data) const {
        return true;
    }

    virtual void decode(const BytesPiece& in, String* out) const {
        out->_bytes.append(in);
    }

    virtual bool at(const BytesPiece& bytes, size_t loc, uint32_t* code) const {
        *code = *reinterpret_cast<const uint32_t*>(
                bytes.substr(loc * sizeof(uint32_t), sizeof(uint32_t)).data());
        return true;
    }

    virtual bool empty(const BytesPiece& bytes) const {
        return bytes.empty();
    }

    virtual size_t size(const BytesPiece& bytes) const {
        return bytes.size() / sizeof(uint32_t);
    }

    virtual StringPiece substr(const BytesPiece& bytes, size_t loc) const {
        return StringPiece(bytes.substr(loc * sizeof(uint32_t)), *this);
    }

    virtual StringPiece substr(const BytesPiece& bytes, size_t loc, size_t size) const {
        return StringPiece(bytes.substr(loc * sizeof(uint32_t), size * sizeof(uint32_t)), *this);
    }

    virtual BytesPiece::const_iterator begin(const BytesPiece& bytes) const {
        return bytes.begin();
    }

    virtual BytesPiece::const_iterator end(const BytesPiece& bytes) const {
        return bytes.end();
    }

    virtual void next(const BytesPiece& bytes, BytesPiece::const_iterator* loc) const {
        static_cast<void>(bytes);
        *loc += 4;
    }

    virtual bool dereference(
            const BytesPiece& bytes, BytesPiece::const_iterator loc, uint32_t* code) const {
        *code = *reinterpret_cast<const uint32_t*>(BytesPiece(loc, loc + 4).data());
        return true;
    }

    virtual bool can_encode(uint32_t code) const {
        return true;
    }

    virtual bool can_encode(const StringPiece& in) const {
        return true;
    }

    virtual void encode(const StringPiece& in, Bytes* out) const {
        foreach (it, in) {
            encode(*it, out);
        }
    }

    virtual void encode(uint32_t code, Bytes* out) const {
        out->append(reinterpret_cast<const uint8_t*>(&code), sizeof(uint32_t));
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(InternalEncoding);
};

// String implementation.

const String::InternalEncoding& String::internal_encoding() {
    static const InternalEncoding instance;
    return instance;
}

String::String() { }

String::String(const String& string)
    : _bytes(string._bytes.data(), string._bytes.size()) { }

String::String(const StringPiece& string) {
    internal_encoding().encode(string, &_bytes);
}

void String::append(const String& string) {
    _bytes.append(string._bytes);
}

void String::append(const StringPiece& string) {
    internal_encoding().encode(string, &_bytes);
}

void String::assign(const String& string) {
    _bytes.assign(string._bytes);
}

void String::assign(const StringPiece& string) {
    _bytes.clear();
    internal_encoding().encode(string, &_bytes);
}

String::String(const char* data, const Encoding& encoding) {
    internal_encoding().encode(StringPiece(data, encoding), &_bytes);
}

void String::append(const char* data, const Encoding& encoding) {
    internal_encoding().encode(StringPiece(data, encoding), &_bytes);
}

void String::assign(const char* data, const Encoding& encoding) {
    _bytes.clear();
    internal_encoding().encode(StringPiece(data, encoding), &_bytes);
}

String::String(const BytesPiece& bytes, const Encoding& encoding) {
    internal_encoding().encode(StringPiece(bytes, encoding), &_bytes);
}

void String::append(const BytesPiece& bytes, const Encoding& encoding) {
    internal_encoding().encode(StringPiece(bytes, encoding), &_bytes);
}

void String::assign(const BytesPiece& bytes, const Encoding& encoding) {
    _bytes.clear();
    internal_encoding().encode(StringPiece(bytes, encoding), &_bytes);
}

String::String(size_t num, uint32_t code) {
    append(num, code);
}

void String::append(size_t num, uint32_t code) {
    if (!is_valid_code_point(code)) {
        abort();
    }
    foreach (i, range(num)) {
        _bytes.append(reinterpret_cast<const uint8_t*>(&code), sizeof(uint32_t));
    }
}

void String::assign(size_t num, uint32_t code) {
    _bytes.clear();
    append(num, code);
}

String::~String() { }

size_t String::size() const {
    return internal_encoding().size(_bytes);
}

uint32_t String::at(size_t loc) const {
    uint32_t code;
    internal_encoding().at(_bytes, loc, &code);
    return code;
}

void String::clear() {
    _bytes.clear();
}

bool String::empty() const {
    return internal_encoding().empty(_bytes);
}

void String::resize(size_t size, uint32_t code) {
    if (size < this->size()) {
        _bytes.resize(sizeof(uint32_t) * size);
    } else if (size > this->size()) {
        append(size - this->size(), code);
    }
}

void String::swap(String* string) {
    _bytes.swap(&string->_bytes);
}

size_t String::find(uint32_t code, size_t index) const {
    foreach (i, range(index, size())) {
        if (at(i) == code) {
            return i;
        }
    }
    return kNone;
}

size_t String::rfind(uint32_t code, size_t index) const {
    if (index == kNone) {
        index = size() - 1;
    }
    foreach (i, range(index + 1)) {
        if (at(index - i) == code) {
            return index - i;
        }
    }
    return kNone;
}

void String::replace(size_t index, size_t num, const StringPiece& string) {
    String tail(StringPiece(*this).substr(index + num));
    resize(index);
    append(string);
    append(tail);
}

/*
StringIterator String::begin() const {
    return StringPiece(*this).begin();
}

StringIterator String::end() const {
    return StringPiece(*this).end();
}
*/

// StringKey implementation.

bool operator<(const StringKey& lhs, const StringKey& rhs) {
    foreach (i, range(min(lhs.size(), rhs.size()))) {
        if (lhs.at(i) < rhs.at(i)) {
            return true;
        } else if (lhs.at(i) > rhs.at(i)) {
            return false;
        }
    }
    return lhs.size() < rhs.size();
}

// StringPiece implementation.

StringPiece::StringPiece()
    : _encoding(&String::internal_encoding()) { }

StringPiece::StringPiece(const String& string)
    : _bytes(string._bytes),
      _encoding(&String::internal_encoding()) { }

StringPiece::StringPiece(const char* data, const Encoding& encoding)
    : _bytes(reinterpret_cast<const uint8_t*>(data), strlen(data)),
      _encoding(&encoding) { }

StringPiece::StringPiece(const BytesPiece& bytes, const Encoding& encoding)
    : _bytes(bytes),
      _encoding(&encoding) { }

size_t StringPiece::size() const {
    return _encoding->size(_bytes);
}

uint32_t StringPiece::at(size_t loc) const {
    uint32_t code;
    if (_encoding->at(_bytes, loc, &code)) {
        return code;
    }
    return kUnknownCodePoint;
}

bool StringPiece::empty() const {
    return _encoding->empty(_bytes);
}

StringPiece StringPiece::substr(size_t loc) const {
    return _encoding->substr(_bytes, loc);
}

StringPiece StringPiece::substr(size_t loc, size_t size) const {
    return _encoding->substr(_bytes, loc, size);
}

StringPiece::const_iterator StringPiece::begin() const {
    return const_iterator(this, _encoding->begin(_bytes));
}

StringPiece::const_iterator StringPiece::end() const {
    return const_iterator(this, _encoding->end(_bytes));
}

// StringPiece::const_iterator implementation.

StringPiece::const_iterator::const_iterator(
        const StringPiece* parent, BytesPiece::const_iterator it)
    : _parent(parent),
      _it(it) { }

StringPiece::const_iterator::value_type StringPiece::const_iterator::operator*() const {
    uint32_t code;
    if (_parent->_encoding->dereference(_parent->_bytes, _it, &code)) {
        return code;
    } else {
        return kUnknownCodePoint;
    }
}

StringPiece::const_iterator& StringPiece::const_iterator::operator++() {
    _parent->_encoding->next(_parent->_bytes, &_it);
    return *this;
}

StringPiece::const_iterator StringPiece::const_iterator::operator++(int) {
    BytesPiece::const_iterator old = _it;
    _parent->_encoding->next(_parent->_bytes, &_it);
    return const_iterator(_parent, old);
}

bool StringPiece::const_iterator::operator==(const StringPiece::const_iterator& it) {
    return _it == it._it;
}

bool StringPiece::const_iterator::operator!=(const StringPiece::const_iterator& it) {
    return _it != it._it;
}

bool operator==(const StringPiece& lhs, const StringPiece& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (StringPiece::const_iterator it = lhs.begin(), jt = rhs.begin();
            it != lhs.end() && jt != rhs.end(); ++it, ++jt) {
        if (*it != *jt) {
            return false;
        }
    }
    return true;
}

bool operator!=(const StringPiece& lhs, const StringPiece& rhs) {
    return !(lhs == rhs);
}

}  // namespace sfz
