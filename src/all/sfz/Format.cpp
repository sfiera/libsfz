// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Format.hpp"

#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"
#include "sfz/String.hpp"
#include "sfz/StringUtilities.hpp"

namespace sfz {

namespace {

void span_complement(
        const StringSlice& input, const StringSlice& chars,
        StringSlice* span, StringSlice* remainder) {
    foreach (int i, range(input.size())) {
        if (chars.find(input.at(i)) != String::npos) {
            *span = input.slice(0, i);
            *remainder = input.slice(i);
            return;
        }
    }
    *span = input;
    *remainder = StringSlice();
}

}  // namespace

void print_format_to(
        PrintTarget out, const char* format_string, const PrintItem* const* items, size_t size) {
    static const StringSlice kBraces = "{}";
    static const StringSlice kCloseBrace = "}";

    StringSlice f = format_string;

    while (f.size() > 0) {
        StringSlice span;
        StringSlice remainder;
        span_complement(f, kBraces, &span, &remainder);
        if (remainder.size() == 1) {
            span = f;
            remainder = StringSlice();
        }
        out.push(span);
        f = remainder;
        if (f.size() > 0) {
            if (f.at(0) == f.at(1)) {
                out.push(1, f.at(0));
                f = f.slice(2);
            } else if (f.at(0) == '}') {
                out.push(1, '}');
                f = f.slice(1);
            } else {
                f = f.slice(1);
                span_complement(f, kCloseBrace, &span, &remainder);

                uint32_t index;
                if (string_to_int(span, &index)) {
                    if (index < size) {
                        items[index]->print_to(out);
                    }
                }

                if (remainder.empty()) {
                    f = remainder;
                } else {
                    f = remainder.slice(1);
                }
            }
        }
    }
}

}  // namespace sfz
