// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/PrintItem.hpp"

#include "sfz/Formatter.hpp"

namespace sfz {

template <>
void PrintItem::Dispatch<void>::print_to(const void* target, PrintTarget out) {
}

template <>
void PrintItem::Dispatch<bool>::print_to(const void* target, PrintTarget out) {
    if (*reinterpret_cast<const bool*>(target)) {
        out.append("true");
    } else {
        out.append("false");
    }
}

template <>
void PrintItem::Dispatch<char>::print_to(const void* target, PrintTarget out) {
    out.append(1, *reinterpret_cast<const char*>(target));
}

template <>
void PrintItem::Dispatch<signed char>::print_to(const void* target, PrintTarget out) {
    signed char value = *reinterpret_cast<const signed char*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<signed short>::print_to(const void* target, PrintTarget out) {
    signed short value = *reinterpret_cast<const signed short*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<signed int>::print_to(const void* target, PrintTarget out) {
    signed int value = *reinterpret_cast<const signed int*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<signed long>::print_to(const void* target, PrintTarget out) {
    signed long value = *reinterpret_cast<const signed long*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<signed long long>::print_to(const void* target, PrintTarget out) {
    signed long long value = *reinterpret_cast<const signed long long*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<unsigned char>::print_to(const void* target, PrintTarget out) {
    unsigned char value = *reinterpret_cast<const unsigned char*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<unsigned short>::print_to(const void* target, PrintTarget out) {
    unsigned short value = *reinterpret_cast<const unsigned short*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<unsigned int>::print_to(const void* target, PrintTarget out) {
    unsigned int value = *reinterpret_cast<const unsigned int*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<unsigned long>::print_to(const void* target, PrintTarget out) {
    unsigned long value = *reinterpret_cast<const unsigned long*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<unsigned long long>::print_to(const void* target, PrintTarget out) {
    unsigned long long value = *reinterpret_cast<const unsigned long long*>(target);
    sfz::print_to(out, dec(value));
}

template <>
void PrintItem::Dispatch<float>::print_to(const void* target, PrintTarget out) {
    float value = *reinterpret_cast<const float*>(target);
    char* string;
    if (asprintf(&string, "%f", value) < 0) {
        fprintf(stderr, "asprintf failed");
    } else {
        out.append(string);
        free(string);
    }
}

template <>
void PrintItem::Dispatch<double>::print_to(const void* target, PrintTarget out) {
    double value = *reinterpret_cast<const double*>(target);
    char* string;
    if (asprintf(&string, "%lf", value) < 0) {
        fprintf(stderr, "asprintf failed");
    } else {
        out.append(string);
        free(string);
    }
}

template <>
void PrintItem::Dispatch<const char*>::print_to(const void* target, PrintTarget out) {
    out.append(*reinterpret_cast<const char* const*>(target));
}

template <>
void PrintItem::Dispatch<const char[]>::print_to(const void* target, PrintTarget out) {
    out.append(reinterpret_cast<const char*>(target));
}

template <>
void PrintItem::Dispatch<const void*>::print_to(const void* target, PrintTarget out) {
    uint64_t addr = *reinterpret_cast<const intptr_t*>(target);
    size_t size = sizeof(const void*) * 2;
    sfz::print_to(out, hex(addr, size));
}

}  // namespace sfz
