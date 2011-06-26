// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/string-utils.hpp>

#include <limits>
#include <sfz/encoding.hpp>
#include <sfz/exception.hpp>
#include <sfz/foreach.hpp>
#include <sfz/string.hpp>

using std::numeric_limits;

namespace sfz {

namespace {

bool get_digit_value(Rune rune, int base, int* digit) {
    if (('0' <= rune) && (rune <= '9')) {
        *digit = rune - '0';
        return (*digit < base);
    } else if (('A' <= rune) && (rune <= 'Z')) {
        *digit = rune - 'A' + 10;
        return (*digit < base);
    } else if (('a' <= rune) && (rune <= 'z')) {
        *digit = rune - 'a' + 10;
        return (*digit < base);
    } else {
        return false;
    }
}

template <typename T> bool is_signed();
template <> bool is_signed<int8_t>() { return true; }
template <> bool is_signed<uint8_t>() { return false; }
template <> bool is_signed<int16_t>() { return true; }
template <> bool is_signed<uint16_t>() { return false; }
template <> bool is_signed<int32_t>() { return true; }
template <> bool is_signed<uint32_t>() { return false; }
template <> bool is_signed<int64_t>() { return true; }
template <> bool is_signed<uint64_t>() { return false; }

}  // namespace

template <typename T>
bool string_to_int(StringSlice s, T& out, int base) {
    bool positive = true;
    T value = 0;

    if (s.size() == 0) {
        return false;
    }
    if (is_signed<T>() && (s.at(0) == '-')) {
        positive = false;
        s = s.slice(1);
    }
    SFZ_FOREACH(Rune r, s, {
        int digit;
        if (!get_digit_value(r, base, &digit)) {
            return false;
        }

        if (positive) {
            if (value > (numeric_limits<T>::max() / 10)) {
                return false;
            }
            value *= 10;
            if ((numeric_limits<T>::max() - value) < static_cast<T>(digit)) {
                return false;
            }
            value += digit;
        } else {
            if (value < (numeric_limits<T>::min() / 10)) {
                return false;
            }
            value *= 10;
            if ((numeric_limits<T>::min() - value) > static_cast<T>(-digit)) {
                return false;
            }
            value -= digit;
        }
    });
    out = value;
    return true;
}

template bool string_to_int<int8_t>(StringSlice s, int8_t& out, int base);
template bool string_to_int<uint8_t>(StringSlice s, uint8_t& out, int base);
template bool string_to_int<int16_t>(StringSlice s, int16_t& out, int base);
template bool string_to_int<uint16_t>(StringSlice s, uint16_t& out, int base);
template bool string_to_int<int32_t>(StringSlice s, int32_t& out, int base);
template bool string_to_int<uint32_t>(StringSlice s, uint32_t& out, int base);
template bool string_to_int<int64_t>(StringSlice s, int64_t& out, int base);
template bool string_to_int<uint64_t>(StringSlice s, uint64_t& out, int base);

CString::CString(const StringSlice& string) {
    if (string.find('\0') != StringSlice::npos) {
        throw Exception("Tried to create CString from string with embedded NUL");
    }
    _bytes.append(utf8::encode(string));
    _bytes.append(1, '\0');
}

char* CString::data() {
    return reinterpret_cast<char*>(_bytes.data());
}

const char* CString::data() const {
    return reinterpret_cast<const char*>(_bytes.data());
}

size_t CString::size() const {
    return _bytes.size() - 1;
}

}  // namespace sfz
