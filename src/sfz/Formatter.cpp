// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Formatter.hpp"

#include "sfz/Foreach.hpp"

namespace sfz {

namespace {

const char kNeedEscape[] = "'\"\\\a\b\f\n\r\t\v";
const char kEscaped[] = "'\"\\abfnrtv";

class EscapedStringPrinter : public FormatItemPrinter {
  public:
    EscapedStringPrinter(const StringPiece& string)
        : _string(string) { }

    virtual void print_to(String* out) const {
        foreach (it, _string) {
            uint32_t ch = *it;
            const char* escape = strchr(kNeedEscape, ch);
            if (escape) {
                out->append(1, '\\');
                out->append(1, kEscaped[escape - kNeedEscape]);
            } else {
                out->append(1, ch);
            }
        }
    }

  private:
    StringPiece _string;
};

class QuotedStringPrinter : public EscapedStringPrinter {
  public:
    QuotedStringPrinter(const StringPiece& string)
        : EscapedStringPrinter(string) { }

    virtual void print_to(String* out) const {
        out->append(1, '"');
        EscapedStringPrinter::print_to(out);
        out->append(1, '"');
    }
};

}  // namespace

FormatItem escape(const StringPiece& string) {
    return FormatItem::make(new EscapedStringPrinter(string));
}

FormatItem quote(const StringPiece& string) {
    return FormatItem::make(new QuotedStringPrinter(string));
}

}  // namespace sfz
