// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_STRING_HPP_
#define SFZ_STRING_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <iterator>
#include <sfz/ScopedPtr.hpp>
#include <sfz/PrintItem.hpp>
#include <sfz/PrintTarget.hpp>
#include <sfz/Rune.hpp>

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
    // STL container types and constants.
    typedef uint32_t value_type;
    typedef uint32_t* pointer;
    typedef uint32_t& reference;
    typedef const uint32_t& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    static const size_type npos;

    typedef uint32_t* iterator;
    typedef const uint32_t* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

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

    // Copies the content of an ASCII-encoded, NUL-terminated byte sequence.
    //
    // The decoded content of `data` either is used to initialize the String, is appended to the
    // String, or replaces the current content of the String, depending on the method which is
    // used.
    //
    // @param [in] data     A NUL-terminated byte sequence with ASCII-encoded code points.
    explicit String(const char* data);
    void append(const char* data);
    void assign(const char* data);

    // Copies the content of a printable object.
    //
    // The content of `object` either is used to initialize the String, is appended to the String,
    // or replaces the current content of the string, depending on the method which is used.
    //
    // @param [in] item     A printable object.
    explicit String(const PrintItem& item);
    void append(const PrintItem& item);
    void assign(const PrintItem& item);

    // Copies a repeated code point.
    //
    // The repeated code point `rune` either is used to initialize the String, is appended to the
    // String, or replaces the current content of the String, depending on the method which is
    // used.  The time complexity of these methods is linear in `num`.
    //
    // @param [in] num      The number of repetitions.
    // @param [in] rune     The code point to repeat.  Must be a valid code point.
    String(size_t num, Rune rune);
    void append(size_t num, Rune rune);
    void assign(size_t num, Rune rune);

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
    // created by this extension will be filled with `rune`.
    //
    // @param [in] size     The desired size of the String.
    // @param [in] rune     If the String is extended, the code point to fill new positions with.
    void resize(size_t size, Rune rune = 0);

    void replace(size_t index, size_t num, const StringPiece& string);

    size_t size() const;
    bool empty() const;
    Rune at(size_t loc) const;
    size_t find(Rune rune, size_t index = 0) const;
    size_t find(const StringPiece& string, size_t index = 0) const;
    size_t rfind(Rune rune, size_t index = npos) const;
    size_t rfind(const StringPiece& string, size_t index = npos) const;
    StringPiece substr(size_t loc) const;
    StringPiece substr(size_t loc, size_t size) const;

    // @returns             STL-like iterators over the StringPiece.
    iterator begin() { return _data.get(); }
    iterator end() { return _data.get() + _size; }
    const_iterator begin() const { return _data.get(); }
    const_iterator end() const { return _data.get() + _size; }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    friend void swap(String& x, String& y);

  private:
    friend class StringPiece;

    void initialize(size_t capacity);

    scoped_array<Rune> _data;
    size_t _size;
    size_t _capacity;

    // Disallow assignment.
    String& operator=(const String&);
};

// References a sequence of Unicode code points.
//
// StringPiece references a sequence of valid Unicode code points.  This is represented as a
// pairing between a BytesPiece and an encoding.  Unlike String, it does not guarantee linear-time
// access to individual code points within the sequence, although, depending on the encoding,
// linear-time access is possible.  Access to ASCII code points, for example, is always constant
// time, whereas access to UTF-8 code points is linear.
class StringPiece {
  public:
    // STL container types and constants.
    typedef uint32_t value_type;
    typedef uint32_t* pointer;
    typedef uint32_t& reference;
    typedef const uint32_t& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    static const size_type npos;

    class iterator;
    typedef iterator const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Initializes the StringPiece to the empty string.
    StringPiece();

    // Initializes the StringPiece with the contents of `string`.
    //
    // @param [in] string   The string to reference.
    StringPiece(const String& string);

    StringPiece(const char* ascii_string);

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
    Rune at(size_t loc) const;

    size_t find(Rune rune, size_t index = 0) const;
    size_t find(const StringPiece& string, size_t index = 0) const;
    size_t rfind(Rune rune, size_t index = npos) const;
    size_t rfind(const StringPiece& string, size_t index = npos) const;

    // @param [in] loc      An index into the code point sequence.  Must be at most size().
    // @param [in] size     The desired number of code points in the returned substring.  If
    //                      provided, the sum of `loc` and `size` must be at most size().
    // @returns             The substring starting at index `loc` and containing `size` code
    //                      points.  If `size` is not provided, the returned substring extends to
    //                      the end of the source string.
    StringPiece substr(size_t loc) const;
    StringPiece substr(size_t loc, size_t size) const;

    // @returns             STL-like iterators over the StringPiece.
    const_iterator begin() const;
    const_iterator end() const;
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    // StringPiece iterator class.
    class iterator {
      public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef StringPiece container_type;
        typedef Rune value_type;
        typedef const Rune* pointer;
        typedef const Rune& reference;
        typedef ptrdiff_t difference_type;

        iterator();

        value_type operator*() const;

        iterator& operator++();
        iterator operator++(int);
        iterator& operator--();
        iterator operator--(int);

        iterator& operator+=(int n);
        iterator operator+(int n) const;
        iterator& operator-=(int n);
        iterator operator-(int n) const;

        difference_type operator-(const iterator& other) const;

        value_type operator[](int n) const { return *(*this + n); }

        bool operator==(const iterator& other) const { return _it == other._it; }
        bool operator!=(const iterator& other) const { return _it != other._it; }
        bool operator<(const iterator& other) const { return _it < other._it; }
        bool operator<=(const iterator& other) const { return _it <= other._it; }
        bool operator>(const iterator& other) const { return _it > other._it; }
        bool operator>=(const iterator& other) const { return _it >= other._it; }

      private:
        friend class StringPiece;
        iterator(const uint8_t* it, int encoding);

        const uint8_t* _it;
        int _encoding;

        // ALLOW_COPY_AND_ASSIGN
    };

    friend void swap(StringPiece& x, StringPiece& y);

  private:
    friend class String;

    StringPiece(const uint8_t* data, size_t size, int encoding);

    const uint8_t* _data;
    int _encoding;
    size_t _size;

    // ALLOW_COPY_AND_ASSIGN
};

inline void print_to(PrintTarget out, const String& s) { out.append(s); }
inline void print_to(PrintTarget out, const StringPiece& s) { out.append(s); }

void swap(String& x, String& y);
void swap(StringPiece& x, StringPiece& y);

bool operator==(const String& lhs, const String& rhs);
bool operator!=(const String& lhs, const String& rhs);
bool operator==(const StringPiece& lhs, const StringPiece& rhs);
bool operator!=(const StringPiece& lhs, const StringPiece& rhs);

}  // namespace sfz

#endif  // SFZ_STRING_HPP_
