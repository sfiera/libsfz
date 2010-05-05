// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Format.hpp"

#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"
#include "sfz/StringUtilities.hpp"

namespace sfz {

void format(String* out, const char* fmt, SFZ_FORMAT_ITEMS_DEFINITION) {
    const PrintItem* items[16] = SFZ_FORMAT_ITEMS_ARRAY;
    FormatResult result(fmt, SFZ_FORMAT_ITEM_COUNT, items);
    result.print_to(out);
}

void print(int fd, const char* fmt, SFZ_FORMAT_ITEMS_DEFINITION) {
    const PrintItem* items[SFZ_FORMAT_ITEM_COUNT] = SFZ_FORMAT_ITEMS_ARRAY;
    String decoded;
    FormatResult(fmt, SFZ_FORMAT_ITEM_COUNT, items).print_to(&decoded);
    Bytes encoded(decoded, utf8_encoding());
    write(fd, encoded.data(), encoded.size());
}

FormatResult::FormatResult(const char* fmt, size_t item_count, const PrintItem** items)
    : _fmt(fmt),
      _item_count(item_count),
      _items(items) { }

namespace {

void span_complement(
        const StringPiece& input, const StringPiece& chars,
        StringPiece* span, StringPiece* remainder) {
    foreach (i, range(input.size())) {
        if (chars.find(input.at(i)) != String::npos) {
            *span = input.substr(0, i);
            *remainder = input.substr(i);
            return;
        }
    }
    *span = input;
    *remainder = StringPiece();
}

}  // namespace

void FormatResult::print_to(String* out) const {
    static const String kBraces("{}", ascii_encoding());
    static const String kCloseBrace("}", ascii_encoding());

    String decoded(_fmt, ascii_encoding());
    StringPiece f = decoded;

    while (f.size() > 0) {
        StringPiece span;
        StringPiece remainder;
        span_complement(f, kBraces, &span, &remainder);
        if (remainder.size() == 1) {
            span = f;
            remainder = StringPiece();
        }
        out->append(span);
        f = remainder;
        if (f.size() > 0) {
            if (f.at(0) == f.at(1)) {
                out->append(1, f.at(0));
                f = f.substr(2);
            } else if (f.at(0) == '}') {
                out->append(1, '}');
                f = f.substr(1);
            } else {
                f = f.substr(1);
                span_complement(f, kCloseBrace, &span, &remainder);

                int32_t index = -1;
                if (string_to_int32_t(span, &index)) {
                    if (index >= 0 && static_cast<uint32_t>(index) < _item_count) {
                        _items[index]->print_to(out);
                    }
                }

                if (remainder.empty()) {
                    f = remainder;
                } else {
                    f = remainder.substr(1);
                }
            }
        }
    }
}

}  // namespace sfz
