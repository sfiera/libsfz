// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_BYTES_HPP_
#define SFZ_BYTES_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <iterator>
#include <sfz/memory.hpp>
#include <sfz/write.hpp>

namespace sfz {

class BytesSlice;

class Bytes {
  public:
    // STL container types and constants.
    typedef uint8_t                                 value_type;
    typedef uint8_t*                                pointer;
    typedef const uint8_t*                          const_pointer;
    typedef uint8_t&                                reference;
    typedef const uint8_t&                          const_reference;

    typedef size_t                                  size_type;
    typedef ptrdiff_t                               difference_type;

    typedef pointer                                 iterator;
    typedef const_pointer                           const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    static const size_type npos = -1;

    Bytes();
    explicit Bytes(const Bytes& bytes);
    explicit Bytes(WriteItem item);
    Bytes(const uint8_t* data, size_t size);
    Bytes(size_t num, uint8_t byte);
    ~Bytes();

    uint8_t* data();
    const uint8_t* data() const;
    size_t size() const;

    void push(const BytesSlice& bytes);
    void push(size_t num, uint8_t byte);

    void append(WriteItem item);
    void append(const uint8_t* data, size_t size);
    void append(size_t num, uint8_t byte);

    void assign(WriteItem item);
    void assign(const uint8_t* data, size_t size);
    void assign(size_t num, uint8_t byte);

    uint8_t at(size_t loc) const;

    size_type find(uint8_t byte) const;
    size_type find(const BytesSlice& bytes) const;
    size_type rfind(uint8_t byte) const;
    size_type rfind(const BytesSlice& bytes) const;

    void clear();

    bool empty() const;

    BytesSlice slice() const;
    BytesSlice slice(size_t index) const;
    BytesSlice slice(size_t index, size_t size) const;

    void replace(size_t index, size_t num, const BytesSlice& bytes);

    void reserve(size_t capacity);

    void resize(size_t size, uint8_t byte = '\0');

    friend void swap(Bytes& a, Bytes& b);

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

class BytesSlice {
  public:
    // STL container types and constants.
    typedef uint8_t                                 value_type;
    typedef uint8_t*                                pointer;
    typedef const uint8_t*                          const_pointer;
    typedef uint8_t&                                reference;
    typedef const uint8_t&                          const_reference;
    typedef size_t                                  size_type;
    typedef ptrdiff_t                               difference_type;

    typedef const_pointer                           iterator;
    typedef const_pointer                           const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    static const size_type npos = -1;

    // Constructors.
    BytesSlice();
    BytesSlice(const Bytes& bytes);
    BytesSlice(const char* data);
    BytesSlice(const uint8_t* data, size_t size);

    const uint8_t* data() const;
    size_t size() const;

    uint8_t at(size_t loc) const;

    size_type find(uint8_t byte) const;
    size_type find(const BytesSlice& bytes) const;
    size_type rfind(uint8_t byte) const;
    size_type rfind(const BytesSlice& bytes) const;

    bool empty() const;

    BytesSlice slice() const;
    BytesSlice slice(size_t index) const;
    BytesSlice slice(size_t index, size_t size) const;

    void shift(size_t size);
    void shift(uint8_t* data, size_t size);

    const_iterator begin() const { return const_iterator(_data); }
    const_iterator end() const { return const_iterator(_data + _size); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    friend void swap(BytesSlice& a, BytesSlice& b);

  private:
    const uint8_t* _data;
    size_t _size;

    // ALLOW_COPY_AND_ASSIGN
};

inline void write_to(WriteTarget out, const Bytes& bytes) { out.push(bytes); }
inline void write_to(WriteTarget out, const BytesSlice& bytes) { out.push(bytes); }

void swap(Bytes& x, Bytes& y);
void swap(BytesSlice& x, BytesSlice& y);

bool operator==(const BytesSlice& x, const BytesSlice& y);
inline bool operator!=(const BytesSlice& x, const BytesSlice& y) { return !(x == y); }
bool operator< (const BytesSlice& x, const BytesSlice& y);
inline bool operator<=(const BytesSlice& x, const BytesSlice& y) { return !(y < x); }
inline bool operator> (const BytesSlice& x, const BytesSlice& y) { return y < x; }
inline bool operator>=(const BytesSlice& x, const BytesSlice& y) { return !(x < y); }

inline bool operator==(const Bytes& x, const Bytes& y) { return x.slice() == y.slice(); }
inline bool operator!=(const Bytes& x, const Bytes& y) { return x.slice() != y.slice(); }
inline bool operator< (const Bytes& x, const Bytes& y) { return x.slice() < y.slice(); }
inline bool operator<=(const Bytes& x, const Bytes& y) { return x.slice() <= y.slice(); }
inline bool operator> (const Bytes& x, const Bytes& y) { return x.slice() > y.slice(); }
inline bool operator>=(const Bytes& x, const Bytes& y) { return x.slice() >= y.slice(); }

}  // namespace sfz

#endif  // SFZ_BYTES_HPP_
