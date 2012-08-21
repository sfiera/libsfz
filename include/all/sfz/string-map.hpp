// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_STRING_MAP_HPP_
#define SFZ_STRING_MAP_HPP_

#include <map>
#include <utility>
#include <stdlib.h>
#include <sfz/foreach.hpp>
#include <sfz/memory.hpp>
#include <sfz/string.hpp>

namespace sfz {

template <typename T, typename Compare = std::less<StringSlice> >
class StringMap {
  public:
    typedef StringSlice                             key_type;
    typedef T                                       mapped_type;
    typedef std::pair<const key_type, mapped_type>  value_type;
    typedef size_t                                  size_type;

    class iterator;
    class const_iterator;

    StringMap() { }
    explicit StringMap(const StringMap& other);
    ~StringMap() { }

    mapped_type& operator[](const key_type& key);
    std::pair<iterator, bool> insert(const value_type& pair);

    size_type size() const { return _map.size(); }
    bool empty() const { return _map.empty(); }

    void clear() { _map.clear(); }
    void erase(iterator pos) { _map.erase(pos); }
    void erase(iterator start, iterator end) { _map.erase(start, end); }
    size_type erase(const key_type& key) { return _map.erase(key); }

    iterator find(const key_type& key) { return _map.find(key); }
    const_iterator find(const key_type& key) const { return _map.find(key); }

    iterator begin() { return _map.begin(); }
    const_iterator begin() const { return _map.begin(); }
    iterator end() { return _map.end(); }
    const_iterator end() const { return _map.end(); }
    iterator rbegin() { return _map.rbegin(); }
    const_iterator rbegin() const { return _map.rbegin(); }
    iterator rend() { return _map.rend(); }
    const_iterator rend() const { return _map.rend(); }

    void swap(StringMap& from) { _map.swap(from._map); }

  private:
    struct WrappedValue;
    typedef std::map<StringSlice, linked_ptr<WrappedValue>, Compare> internal_map;
    typedef typename internal_map::iterator wrapped_iterator;
    typedef typename internal_map::const_iterator wrapped_const_iterator;

    struct WrappedValue {
        const String key_storage;
        std::pair<const StringSlice, mapped_type> pair;

        WrappedValue(const StringSlice& k)
            : key_storage(k),
              pair(key_storage, mapped_type()) { }

        WrappedValue(const StringSlice& k, const mapped_type& v)
            : key_storage(k),
              pair(key_storage, v) { }

        DISALLOW_COPY_AND_ASSIGN(WrappedValue);
    };

    template <typename wrapped_iterator>
    class iterator_base {
      public:
        typedef typename wrapped_iterator::iterator_category    iterator_category;
        typedef typename StringMap::value_type                  value_type;
        typedef typename wrapped_iterator::difference_type      difference_type;
        typedef value_type*                                     pointer;
        typedef value_type&                                     reference;

        iterator_base() { }
        iterator_base(wrapped_iterator it) : _it(it) { }

        reference operator*() const { return _it->second->pair; }
        pointer operator->() const { return &_it->second->pair; }

        iterator_base& operator++() { ++_it; return *this; }
        iterator_base operator++(int) { return _it++; }
        iterator_base& operator--() { --_it; return *this; }
        iterator_base operator--(int) { return _it--; }

        bool operator==(iterator_base it) { return _it == it._it; }
        bool operator!=(iterator_base it) { return _it != it._it; }

      private:
        wrapped_iterator _it;
    };

    internal_map _map;

    StringMap& operator=(const StringMap&);  // DISALLOW_ASSIGN
};

template <typename T, typename Compare>
bool operator==(const StringMap<T, Compare>& x, const StringMap<T, Compare>& y) {
    typedef typename StringMap<T, Compare>::const_iterator iterator;
    iterator x_begin = x.begin(), x_end = x.end(), y_begin = y.begin(), y_end = y.end();
    while ((x_begin != x_end) && (y_begin != y_end)) {
        if ((x_begin->first != y_begin->first) || (x_begin->second != y_begin->second)) {
            return false;
        }
        ++x_begin;
        ++y_begin;
    }
    return (x_begin == x_end) && (y_begin == y_end);
}

template <typename T, typename Compare>
bool operator!=(const StringMap<T, Compare>& x, const StringMap<T, Compare>& y) {
    return !(x == y);
}

template <typename T, typename Compare>
class StringMap<T, Compare>::iterator : public iterator_base<wrapped_iterator> {
  public:
    iterator() { }
    iterator(const_iterator it) : iterator_base<wrapped_iterator>(it._it) { }

  private:
    friend class StringMap;
    iterator(wrapped_iterator it) : iterator_base<wrapped_iterator>(it) { }
};

template <typename T, typename Compare>
class StringMap<T, Compare>::const_iterator : public iterator_base<wrapped_const_iterator> {
  public:
    const_iterator() { }

  private:
    friend class StringMap;
    friend class iterator;
    const_iterator(wrapped_const_iterator it) : iterator_base<wrapped_const_iterator>(it) { }
};

template <typename T, typename Compare>
StringMap<T, Compare>::StringMap(const StringMap& other) {
    SFZ_FOREACH(const value_type& item, other, {
        insert(item);
    });
}

template <typename T, typename Compare>
typename StringMap<T, Compare>::mapped_type& StringMap<T, Compare>::operator[](
        const key_type& key) {
    wrapped_iterator it = _map.find(key);
    if (it == _map.end()) {
        linked_ptr<WrappedValue> inserted(new WrappedValue(key));
        _map.insert(typename internal_map::value_type(inserted->key_storage, inserted));
        return inserted->pair.second;
    }
    return it->second->pair.second;
}

template <typename T, typename Compare>
std::pair<typename StringMap<T, Compare>::iterator, bool> StringMap<T, Compare>::insert(
        const value_type& pair) {
    const StringSlice& key = pair.first;
    const mapped_type& value = pair.second;
    wrapped_iterator it = _map.find(key);
    if (it == _map.end()) {
        linked_ptr<WrappedValue> inserted(new WrappedValue(key, value));
        it = _map.insert(typename internal_map::value_type(inserted->key_storage, inserted)).first;
        return std::make_pair(iterator(it), true);
    } else {
        return std::make_pair(iterator(it), false);
    }
}

}  // namespace sfz

#endif  // SFZ_STRING_MAP_HPP_
