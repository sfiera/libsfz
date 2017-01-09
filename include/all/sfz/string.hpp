// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_STRING_HPP_
#define SFZ_STRING_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sfz/print.hpp>

namespace sfz {

class StringSlice;

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
    typedef Rune        value_type;
    typedef Rune*       pointer;
    typedef const Rune* const_pointer;
    typedef Rune&       reference;
    typedef const Rune& const_reference;

    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;

    typedef pointer                               iterator;
    typedef const_pointer                         const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos = -1;

    // Initializes the String to the empty string.
    String();

    // Copies the content of another String.
    //
    // The content of `string` either is used to initialize the String, is appended to the String,
    // or replaces the current content of the String, depending on the method which is used.  The
    // time complexity of these methods is linear in the length of `string`.
    //
    // @param [in] string   The string to copy.
    explicit String(const String& string);

    String(String&& string) = default;
    String& operator=(String&& string) = default;
    void assign(String&& string);

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

    void push(const StringSlice& string);
    void push(size_t num, Rune rune);

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

    void replace(size_t index, size_t num, const StringSlice& string);

    size_t size() const;
    bool   empty() const;
    Rune at(size_t loc) const;
    size_t find(Rune rune, size_t index = 0) const;
    size_t find(const StringSlice& string, size_t index = 0) const;
    size_t rfind(Rune rune, size_t index = npos) const;
    size_t rfind(const StringSlice& string, size_t index = npos) const;

    StringSlice slice() const;
    StringSlice slice(size_t loc) const;
    StringSlice slice(size_t loc, size_t size) const;

    // @returns             STL-like iterators over the StringSlice.
    iterator               begin() { return _data.get(); }
    iterator               end() { return _data.get() + _size; }
    const_iterator         begin() const { return _data.get(); }
    const_iterator         end() const { return _data.get() + _size; }
    reverse_iterator       rbegin() { return reverse_iterator(end()); }
    reverse_iterator       rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    friend void swap(String& x, String& y);

  private:
    friend class StringSlice;
    friend bool operator==(const String& x, const String& y);
    friend bool operator<(const String& x, const String& y);

    void initialize(size_t capacity);

    std::unique_ptr<Rune[]> _data;
    size_t                  _size;
    size_t                  _capacity;

    // Disallow assignment.
    String& operator=(const String&);
};

// References a sequence of Unicode code points.
//
// StringSlice references a sequence of valid Unicode code points.  This is represented as a
// pairing between a BytesSlice and an encoding.  Unlike String, it does not guarantee linear-time
// access to individual code points within the sequence, although, depending on the encoding,
// linear-time access is possible.  Access to ASCII code points, for example, is always constant
// time, whereas access to UTF-8 code points is linear.
class StringSlice {
  public:
    // STL container types and constants.
    typedef Rune        value_type;
    typedef Rune*       pointer;
    typedef const Rune* const_pointer;
    typedef Rune&       reference;
    typedef const Rune& const_reference;

    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;

    class iterator;
    typedef iterator                              const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static const size_type npos = -1;

    // Initializes the StringSlice to the empty string.
    StringSlice();

    // Initializes the StringSlice with the contents of `string`.
    //
    // @param [in] string   The string to reference.
    StringSlice(const String& string);

    StringSlice(const char* ascii_string);

    // Accessors.
    //
    // Typically, size(), at(), and slice() will run in constant time when the StringSlice is
    // encoded using a fixed-width encoding (e.g. ASCII or UCS-2), and in linear time when encoded
    // using a variable-width encoding (e.g. UTF-8 or UTF-16).  As a result, it is typically a much
    // better idea to loop over a StringSlice using iterators than indices.
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
    size_t find(const StringSlice& string, size_t index = 0) const;
    size_t rfind(Rune rune, size_t index = npos) const;
    size_t rfind(const StringSlice& string, size_t index = npos) const;

    // @param [in] loc      An index into the code point sequence.  Must be at most size().
    // @param [in] size     The desired number of code points in the returned slice.  If provided,
    //                      the sum of `loc` and `size` must be at most size().
    // @returns             The slice starting at index `loc` and containing `size` code points. If
    //                      `size` is not provided, the returned slice extends to the end of the
    //                      source string.
    StringSlice slice() const;
    StringSlice slice(size_t loc) const;
    StringSlice slice(size_t loc, size_t size) const;

    // @returns             STL-like iterators over the StringSlice.
    const_iterator         begin() const;
    const_iterator         end() const;
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    // StringSlice iterator class.
    class iterator {
      public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef StringSlice                     container_type;
        typedef Rune                            value_type;
        typedef const Rune*                     pointer;
        typedef const Rune&                     reference;
        typedef ptrdiff_t                       difference_type;

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

      private:
        friend class StringSlice;
        friend bool operator==(const StringSlice::iterator& x, const StringSlice::iterator& y);
        friend bool operator<(const StringSlice::iterator& x, const StringSlice::iterator& y);
        iterator(const uint8_t* it, int encoding);

        const uint8_t* _it;
        int            _encoding;

        // ALLOW_COPY_AND_ASSIGN
    };

    friend void swap(StringSlice& x, StringSlice& y);

  private:
    friend class String;

    StringSlice(const uint8_t* data, size_t size, int encoding);

    const uint8_t* _data;
    int            _encoding;
    size_t         _size;

    // ALLOW_COPY_AND_ASSIGN
};

inline void print_to(PrintTarget out, const String& s) { out.push(s); }
inline void print_to(PrintTarget out, const StringSlice& s) { out.push(s); }

void swap(String& x, String& y);
void swap(StringSlice& x, StringSlice& y);

inline void copy(String& to, const String& from) { to.assign(from); }

inline bool store_argument(String& to, StringSlice from, PrintTarget error) {
    to.assign(from);
    return true;
}

bool operator==(const String& x, const String& y);
inline bool operator!=(const String& x, const String& y) { return !(x == y); }
bool operator<(const String& x, const String& y);
inline bool operator<=(const String& x, const String& y) { return !(y < x); }
inline bool operator>(const String& x, const String& y) { return y < x; }
inline bool operator>=(const String& x, const String& y) { return !(x < y); }

bool operator==(const StringSlice& x, const StringSlice& y);
inline bool operator!=(const StringSlice& x, const StringSlice& y) { return !(x == y); }
bool operator<(const StringSlice& x, const StringSlice& y);
inline bool operator<=(const StringSlice& x, const StringSlice& y) { return !(y < x); }
inline bool operator>(const StringSlice& x, const StringSlice& y) { return y < x; }
inline bool operator>=(const StringSlice& x, const StringSlice& y) { return !(x < y); }

inline bool operator==(const StringSlice::iterator& x, const StringSlice::iterator& y) {
    return x._it == y._it;
}
inline bool operator!=(const StringSlice::iterator& x, const StringSlice::iterator& y) {
    return !(x == y);
}
inline bool operator<(const StringSlice::iterator& x, const StringSlice::iterator& y) {
    return x._it < y._it;
}
inline bool operator<=(const StringSlice::iterator& x, const StringSlice::iterator& y) {
    return !(y < x);
}
inline bool operator>(const StringSlice::iterator& x, const StringSlice::iterator& y) {
    return y < x;
}
inline bool operator>=(const StringSlice::iterator& x, const StringSlice::iterator& y) {
    return !(x < y);
}

}  // namespace sfz

#endif  // SFZ_STRING_HPP_
