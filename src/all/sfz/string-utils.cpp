// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/string-utils.hpp>

#include <wctype.h>
#include <limits>
#include <pn/string>
#include <sfz/encoding.hpp>
#include <sfz/string.hpp>
#include <stdexcept>

using std::numeric_limits;

namespace sfz {

void upper(String& s) {
    for (String::iterator begin = s.begin(), end = s.end(); begin != end; ++begin) {
        if (*begin <= Rune(std::numeric_limits<wchar_t>::max())) {
            *begin = towupper(*begin);
        }
    }
}

void lower(String& s) {
    for (String::iterator begin = s.begin(), end = s.end(); begin != end; ++begin) {
        if (*begin <= Rune(std::numeric_limits<wchar_t>::max())) {
            *begin = towlower(*begin);
        }
    }
}

pn::string upper(pn::string_view s) {
    pn::string out;
    for (pn::rune r : s) {
        out += pn::rune(towupper(r.value()));
    }
    return out;
}

pn::string lower(pn::string_view s) {
    pn::string out;
    for (pn::rune r : s) {
        out += pn::rune(towlower(r.value()));
    }
    return out;
}

CString::CString(const StringSlice& string) {
    if (string.find('\0') != StringSlice::npos) {
        throw std::runtime_error("Tried to create CString from string with embedded NUL");
    }
    _bytes.append(utf8::encode(string));
    _bytes.append(1, '\0');
}

char* CString::data() { return reinterpret_cast<char*>(_bytes.data()); }

const char* CString::data() const { return reinterpret_cast<const char*>(_bytes.data()); }

size_t CString::size() const { return _bytes.size() - 1; }

}  // namespace sfz
