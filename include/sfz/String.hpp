// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_STRING_HPP_
#define SFZ_STRING_HPP_

#include <stdint.h>
#include <stdlib.h>
#include "sfz/Macros.hpp"
#include "sfz/Bytes.hpp"
#include "sfz/Encoding.hpp"

namespace sfz {

class StringPiece;

// Stores a sequence of Unicode code points.
//
// Only valid Unicode code points are contained in String objects: validity means that the code
// point is in the range [U+0, U+D7FF] or in [U+E000, U+10FFFF].  Code points in the surrogate code
// range [U+D800, U+DFFF] are disallowed, as are code points outside of the 17 defined planes.
// This means, in particular, that String is not an appropriate container for binary data; use the
// Bytes class for that.
class String {
  public:
    static const size_t kNone = -1;

    // Initializes the String to the empty string.
    String();

    // Copies the content of another String or StringPiece.
    //
    // The content of `string` either is used to initialize the String, is appended to the String,
    // or replaces the current content of the String, depending on the method which is used.  The
    // time complexity of these methods is linear in the length of `string`.
    //
    // @param [in] string   The string to copy.
    explicit String(const String& string);
    explicit String(const StringPiece& string);
    void append(const String& string);
    void append(const StringPiece& string);
    void assign(const String& string);
    void assign(const StringPiece& string);

    // Copies the content of an encoded NUL-terminated byte sequence.
    //
    // The decoded content of `data` either is used to initialize the String, is appended to the
    // String, or replaces the current content of the String, depending on the method which is
    // used.  The time complexity of these methods is dependent on `encoding`.
    //
    // @param [in] data     A NUL-terminated byte sequence with encoded code points.
    // @param [in] encoding The encoding to use to decode `data`.
    String(const char* data, const Encoding& encoding);
    void append(const char* data, const Encoding& encoding);
    void assign(const char* data, const Encoding& encoding);

    // Copies the content of an encoded byte sequence.
    //
    // The decoded content of `bytes` either is used to initialize the String, is appended to the
    // String, or replaces the current content of the String, depending on the method which is
    // used.  The time complexity of these methods is dependent on `encoding`.
    //
    // @param [in] bytes    A byte sequence with encoded code points.
    // @param [in] encoding The encoding to use to decode `bytes`.
    String(const BytesPiece& bytes, const Encoding& encoding);
    void append(const BytesPiece& bytes, const Encoding& encoding);
    void assign(const BytesPiece& bytes, const Encoding& encoding);

    // Copies a repeated code point.
    //
    // The repeated code point `code` either is used to initialize the String, is appended to the
    // String, or replaces the current content of the String, depending on the method which is
    // used.  The time complexity of these methods is linear in `num`.
    //
    // @param [in] num      The number of repetitions.
    // @param [in] code     The code point to repeat.  Must be a valid code point.
    String(size_t num, uint32_t code);
    void append(size_t num, uint32_t code);
    void assign(size_t num, uint32_t code);

    // Destroys the String.
    ~String();

    // Clears the content of this object.
    void clear();

    // Ensures that there is enough space for `capacity` code points.
    //
    // @param [in] capacity The number of code points to reserve space for.
    void reserve(size_t capacity);

    // Resizes the string to the specified size.
    //
    // If `size` is less than the current size of the String, the String will be truncated.  If it
    // is greater, then the String will be extended to the specified size, and the new positions
    // created by this extension will be filled with `code`.
    //
    // @param [in] size     The desired size of the String.
    // @param [in] code     If the String is extended, the code point to fill new positions with.
    void resize(size_t size, uint32_t code = 0);

    // Swaps the contents of two String objects.
    //
    // This is significantly more efficient than the naive approach.
    //
    // @param [in, out] string The string to swap with.
    void swap(String* string);

    void replace(size_t index, size_t num, const StringPiece& string);

    size_t size() const;
    bool empty() const;
    uint32_t at(size_t loc) const;
    size_t find(uint32_t code, size_t index = 0) const;
    size_t find(const StringPiece& string, size_t index = 0) const;
    size_t rfind(uint32_t code, size_t index = kNone) const;
    size_t rfind(const StringPiece& string, size_t index = kNone) const;
    StringPiece substr(size_t loc) const;
    StringPiece substr(size_t loc, size_t size) const;

    // const_iterator begin() const;
    // const_iterator end() const;

  private:
    friend class StringPiece;

    void initialize(size_t capacity);

    scoped_array<uint32_t> _data;
    size_t _size;
    size_t _capacity;

    // Disallow assignment.
    String& operator=(const String&);
};

// String variant suitable for use in std::map and std::set.
//
// Adds to String the necessary features for use as the key of a std::map, or the value of a
// std::set.  It is implicitly copyable, assignable, and defines a 'less than' operator for
// comparison.  It should not be used for any other purpose.
class StringKey : public String {
  public:
    StringKey(const String& string) : String(string) { }
    StringKey(const StringKey& string) : String(string) { }
    StringKey(const StringPiece& string) : String(string) { }
    StringKey(const char* data, const Encoding& encoding) : String(data, encoding) { }
    StringKey(const BytesPiece& bytes, const Encoding& encoding) : String(bytes, encoding) { }
    StringKey(size_t num, uint32_t code) : String(num, code) { }

    StringKey& operator=(const String& string) { assign(string); return *this; }
    StringKey& operator=(const StringKey& string) { assign(string); return *this; }
    StringKey& operator=(const StringPiece& string) { assign(string); return *this; }

  private:
    // ALLOW_COPY_AND_ASSIGN
};

bool operator<(const StringKey& lhs, const StringKey& rhs);

// References a sequence of Unicode code points.
//
// StringPiece references a sequence of valid Unicode code points.  This is represented as a
// pairing between a BytesPiece and an encoding.  Unlike String, it does not guarantee linear-time
// access to individual code points within the sequence, although, depending on the encoding,
// linear-time access is possible.  Access to ASCII code points, for example, is always constant
// time, whereas access to UTF-8 code points is linear.
class StringPiece {
  public:
    static const size_t kNone = -1;

    class const_iterator;

    // Initializes the StringPiece to the empty string.
    StringPiece();

    // Initializes the StringPiece with the contents of `string`.
    //
    // @param [in] string   The string to reference.
    StringPiece(const String& string);

    // Accessors.
    //
    // Typically, size(), at(), and substr() will run in constant time when the StringPiece is
    // encoded using a fixed-width encoding (e.g. ASCII or UCS-2), and in linear time when encoded
    // using a variable-width encoding (e.g. UTF-8 or UTF-16).  As a result, it is typically a much
    // better idea to loop over a StringPiece using iterators than indices.
    //
    // It is expected that empty() will run in constant time, and iterating over the string will
    // take linear time.  These are, regardless, dependent on the encoding.

    // @returns             The number of code points.
    size_t size() const;

    // @returns             true iff size() == 0.
    bool empty() const;

    // @param [in] loc      An index into the code point sequence.  Must be less than size().
    // @returns             The code point at index `loc`.
    uint32_t at(size_t loc) const;

    size_t find(uint32_t code, size_t index = 0) const;
    size_t find(const StringPiece& string, size_t index = 0) const;
    size_t rfind(uint32_t code, size_t index = kNone) const;
    size_t rfind(const StringPiece& string, size_t index = kNone) const;

    // @param [in] loc      An index into the code point sequence.  Must be at most size().
    // @param [in] size     The desired number of code points in the returned substring.  If
    //                      provided, the sum of `loc` and `size` must be at most size().
    // @returns             The substring starting at index `loc` and containing `size` code
    //                      points.  If `size` is not provided, the returned substring extends to
    //                      the end of the source string.
    StringPiece substr(size_t loc) const;
    StringPiece substr(size_t loc, size_t size) const;

    // @returns             STL-like iterators to the beginning and end of the StringPiece.
    const_iterator begin() const;
    const_iterator end() const;

    // StringPiece iterator class.
    class const_iterator {
      public:
        typedef StringPiece container_type;
        typedef uint32_t value_type;

        value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        bool operator==(const const_iterator& it);
        bool operator!=(const const_iterator& it);

      private:
        friend class StringPiece;
        explicit const_iterator(const uint32_t* it);

        const uint32_t* _it;

        // ALLOW_COPY_AND_ASSIGN
    };

  private:
    friend class String;

    StringPiece(const uint32_t* data, size_t size);

    const uint32_t* _data;
    size_t _size;

    // ALLOW_COPY_AND_ASSIGN
};

bool operator==(const String& lhs, const String& rhs);
bool operator!=(const String& lhs, const String& rhs);
bool operator==(const StringPiece& lhs, const StringPiece& rhs);
bool operator!=(const StringPiece& lhs, const StringPiece& rhs);

}  // namespace sfz

#endif  // SFZ_STRING_HPP_
