// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/CString.hpp"

#include "sfz/Encoding.hpp"
#include "sfz/Exception.hpp"

namespace sfz {

CString::CString(const StringSlice& string) {
    if (string.find('\0') != StringSlice::npos) {
        throw Exception("Tried to create CString from string with embedded NUL");
    }
    _bytes.append(utf8::encode(string));
    _bytes.append(1, '\0');
}

const char* CString::data() const {
    return reinterpret_cast<const char*>(_bytes.data());
}

char* CString::mutable_data() {
    return const_cast<char*>(data());
}

size_t CString::size() const {
    return _bytes.size() - 1;
}

}  // namespace sfz
