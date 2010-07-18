// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_BYTES_HPP_
#define SFZ_BYTES_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <iterator>
#include <sfz/SmartPtr.hpp>
#include <sfz/WriteItem.hpp>
#include <sfz/WriteTarget.hpp>

namespace sfz {

class BytesPiece;

class Bytes {
  public:
    // STL container types and constants.
    typedef uint8_t value_type;
    typedef uint8_t* pointer;
    typedef uint8_t& reference;
    typedef const uint8_t& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    static const size_type npos;

    typedef uint8_t* iterator;
    typedef const uint8_t* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    Bytes();
    explicit Bytes(const Bytes& bytes);
    explicit Bytes(const BytesPiece& bytes);
    Bytes(const uint8_t* data, size_t size);
    explicit Bytes(WriteItem item);
    Bytes(size_t num, uint8_t byte);
    ~Bytes();

    const uint8_t* data() const;
    uint8_t* mutable_data() const;
    size_t size() const;

    void append(const BytesPiece& bytes);
    void append(const uint8_t* data, size_t size);
    void append(WriteItem item);
    void append(size_t num, uint8_t byte);

    void assign(const BytesPiece& bytes);
    void assign(const uint8_t* data, size_t size);
    void assign(WriteItem item);
    void assign(size_t num, uint8_t byte);

    uint8_t at(size_t loc) const;
    uint8_t front() const { return at(0); }

    void clear();

    bool empty() const;

    void reserve(size_t capacity);

    void resize(size_t size, uint8_t byte = '\0');

    void swap(Bytes* bytes);

    // Iterator support.
    iterator begin() { return iterator(_data.get()); }
    iterator end() { return iterator(_data.get() + _size); }
    const_iterator begin() const { return const_iterator(_data.get()); }
    const_iterator end() const { return const_iterator(_data.get() + _size); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

  private:
    scoped_array<uint8_t> _data;
    size_t _size;
    size_t _capacity;

    // Disallow assignment.
    Bytes& operator=(const Bytes&);
};

class BytesPiece {
  public:
    // STL container types and constants.
    typedef uint8_t value_type;
    typedef const uint8_t* pointer;
    typedef const uint8_t& reference;
    typedef const uint8_t& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    static const size_type npos;

    typedef const uint8_t* iterator;
    typedef const uint8_t* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Constructors.
    BytesPiece();
    BytesPiece(const Bytes& bytes);
    BytesPiece(const char* data);
    BytesPiece(const uint8_t* data, size_t size);

    const uint8_t* data() const;
    size_t size() const;

    uint8_t at(size_t loc) const;
    uint8_t front() const { return at(0); }

    bool empty() const;

    BytesPiece substr(size_t index) const;
    BytesPiece substr(size_t index, size_t size) const;

    void shift(size_t size);
    void shift(uint8_t* data, size_t size);

    const_iterator begin() const { return const_iterator(_data); }
    const_iterator end() const { return const_iterator(_data + _size); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

  private:
    const uint8_t* _data;
    size_t _size;

    // ALLOW_COPY_AND_ASSIGN
};

inline void write_to(WriteTarget out, const Bytes& bytes) { out.append(bytes); }
inline void write_to(WriteTarget out, const BytesPiece& bytes) { out.append(bytes); }

bool operator==(const Bytes& lhs, const Bytes& rhs);
bool operator!=(const Bytes& lhs, const Bytes& rhs);

bool operator==(const BytesPiece& lhs, const BytesPiece& rhs);
bool operator!=(const BytesPiece& lhs, const BytesPiece& rhs);

}  // namespace sfz

#endif  // SFZ_BYTES_HPP_
