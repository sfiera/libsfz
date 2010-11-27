// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/StringUtilities.hpp"

#include <limits>
#include "sfz/Foreach.hpp"
#include "sfz/String.hpp"

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
bool string_to_int(const StringSlice& s, T* out, int base) {
    bool positive = true;
    T value = 0;

    if (s.size() == 0) {
        return false;
    }
    foreach (it, s) {
        if (is_signed<T>() && (*it == '-') && (it == s.begin())) {
            positive = false;
            continue;
        }

        int digit;
        if (!get_digit_value(*it, base, &digit)) {
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
    }
    *out = value;
    return true;
}

template bool string_to_int<int8_t>(const StringSlice& s, int8_t* out, int base);
template bool string_to_int<uint8_t>(const StringSlice& s, uint8_t* out, int base);
template bool string_to_int<int16_t>(const StringSlice& s, int16_t* out, int base);
template bool string_to_int<uint16_t>(const StringSlice& s, uint16_t* out, int base);
template bool string_to_int<int32_t>(const StringSlice& s, int32_t* out, int base);
template bool string_to_int<uint32_t>(const StringSlice& s, uint32_t* out, int base);
template bool string_to_int<int64_t>(const StringSlice& s, int64_t* out, int base);
template bool string_to_int<uint64_t>(const StringSlice& s, uint64_t* out, int base);

}  // namespace sfz
