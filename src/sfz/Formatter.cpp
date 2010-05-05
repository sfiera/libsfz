// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Formatter.hpp"

#include "sfz/Foreach.hpp"

namespace sfz {

namespace {

// Maps control characters to their octal-escaped equivalents.  There are two things of note here:
// first, we always specify all three octal digits.  This is to prevent cases where a character
// following the escaped character could be interpreted as part of the escape sequence.  For
// example, we always escape the pair of runes ('\0', '1') as '\0001'.  Second, we use the octal
// equivalents of '\a', '\b', and '\v' rather than the symbolic names.  These characters are rare
// enough that it is considered more informative to provide the numeric name.
const char kEscaped[' '][5] = {
    "\\000", "\\001", "\\002", "\\003", "\\004", "\\005", "\\006", "\\007",
    "\\010", "\\t",   "\\n",   "\\013", "\\014", "\\r",   "\\016", "\\017",
    "\\020", "\\021", "\\022", "\\023", "\\024", "\\025", "\\026", "\\027",
    "\\030", "\\031", "\\032", "\\033", "\\034", "\\035", "\\036", "\\037",
};

class EscapedStringPrinter : public PrintItem::Impl {
  public:
    EscapedStringPrinter(const StringPiece& string)
        : _string(string) { }

    virtual void print_to(String* out) const {
        foreach (it, _string) {
            Rune rune = *it;
            if (rune < ' ') {
                out->append(kEscaped[rune]);
            } else if (rune == '\'' || rune == '\"' || rune == '\\') {
                out->append(1, '\\');
                out->append(1, rune);
            } else {
                out->append(1, rune);
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

PrintItem escape(const StringPiece& string) {
    return PrintItem::make(new EscapedStringPrinter(string));
}

PrintItem quote(const StringPiece& string) {
    return PrintItem::make(new QuotedStringPrinter(string));
}

}  // namespace sfz
