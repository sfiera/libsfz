// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_BYTES_HPP_
#define SFZ_BYTES_HPP_

#include <stdint.h>
#include <stdlib.h>
#include "sfz/Macros.hpp"
#include "sfz/SmartPtr.hpp"

namespace sfz {

class BytesPiece;
class Encoding;
class StringPiece;

class Bytes {
  public:
    Bytes();
    explicit Bytes(const Bytes& bytes);
    explicit Bytes(const BytesPiece& bytes);
    Bytes(const uint8_t* data, size_t size);
    Bytes(const StringPiece& string, const Encoding& encoding);
    Bytes(size_t num, uint8_t byte);
    ~Bytes();

    const uint8_t* data() const;
    uint8_t* mutable_data() const;
    size_t size() const;

    void append(const BytesPiece& bytes);
    void append(const uint8_t* data, size_t size);
    void append(const StringPiece& string, const Encoding& encoding);
    void append(size_t num, uint8_t byte);

    void assign(const BytesPiece& bytes);
    void assign(const uint8_t* data, size_t size);
    void assign(const StringPiece& string, const Encoding& encoding);
    void assign(size_t num, uint8_t byte);

    uint8_t at(size_t loc) const;

    void clear();

    bool empty() const;

    void reserve(size_t capacity);

    void resize(size_t size, uint8_t byte = '\0');

    void swap(Bytes* bytes);

  private:
    scoped_array<uint8_t> _data;
    size_t _size;
    size_t _capacity;

    // Disallow assignment.
    Bytes& operator=(const Bytes&);
};

class BytesPiece {
  public:
    class const_iterator;

    BytesPiece();
    BytesPiece(const Bytes& bytes);
    BytesPiece(const uint8_t* data, size_t size);
    BytesPiece(const_iterator begin, const_iterator end);

    const uint8_t* data() const;
    size_t size() const;

    uint8_t at(size_t loc) const;

    bool empty() const;

    BytesPiece substr(size_t index) const;
    BytesPiece substr(size_t index, size_t size) const;

    // @returns             STL-like iterators to the beginning and end of the BytesPiece.
    const_iterator begin() const;
    const_iterator end() const;

    // BytesPiece iterator class.
    class const_iterator {
      public:
        typedef BytesPiece container_type;
        typedef uint32_t value_type;

        value_type operator*() const;
        value_type operator[](int n) const;

        const_iterator& operator++();
        const_iterator operator++(int);
        const_iterator& operator--();
        const_iterator operator--(int);

        const_iterator operator+(int n);
        const_iterator& operator+=(int n);
        const_iterator operator-(int n);
        const_iterator& operator-=(int n);

        bool operator==(const const_iterator& it);
        bool operator!=(const const_iterator& it);

      private:
        friend class BytesPiece;
        const_iterator(const BytesPiece* parent, const uint8_t* at);

        const BytesPiece* _parent;
        const uint8_t* _it;

        // ALLOW_COPY_AND_ASSIGN
    };

  private:
    const uint8_t* _data;
    size_t _size;

    // ALLOW_COPY_AND_ASSIGN
};

}  // namespace sfz

#endif  // SFZ_BYTES_HPP_
