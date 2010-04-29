// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Format.hpp"

#include "sfz/Foreach.hpp"
#include "sfz/Range.hpp"
#include "sfz/StringUtilities.hpp"

namespace sfz {

namespace {

class EncodedStringPrinter : public FormatItemPrinter {
  public:
    EncodedStringPrinter(const char* string, const Encoding& encoding)
        : _string(string),
          _encoding(encoding) { }

    virtual void print_to(String* out) const {
        out->append(_string, _encoding);
    }

  private:
    const char* const _string;
    const Encoding& _encoding;

    DISALLOW_COPY_AND_ASSIGN(EncodedStringPrinter);
};

class StringPiecePrinter : public FormatItemPrinter {
  public:
    StringPiecePrinter(const StringPiece& string)
        : _string(string) { }

    virtual void print_to(String* out) const {
        out->append(_string);
    }

  private:
    StringPiece _string;

    DISALLOW_COPY_AND_ASSIGN(StringPiecePrinter);
};

class BooleanPrinter : public FormatItemPrinter {
  public:
    BooleanPrinter(bool b)
        : _value(b) { }

    virtual void print_to(String* out) const {
        if (_value) {
            out->append("true", ascii_encoding());
        } else {
            out->append("false", ascii_encoding());
        }
    }

  private:
    bool _value;

    DISALLOW_COPY_AND_ASSIGN(BooleanPrinter);
};

class CharacterPrinter : public FormatItemPrinter {
  public:
    CharacterPrinter(char ch)
        : _value(ch) { }

    virtual void print_to(String* out) const {
        out->append(1, _value);
    }

  private:
    char _value;

    DISALLOW_COPY_AND_ASSIGN(CharacterPrinter);
};

class PrintfPrinter : public FormatItemPrinter {
  public:
    ~PrintfPrinter() {
        if (_value) {
            free(_value);
        }
    }

    virtual void print_to(String* out) const {
        out->append(_value, ascii_encoding());
    }

  protected:
    template <typename T>
    PrintfPrinter(const char* format, T value) {
        if (asprintf(&_value, format, value) < 0) {
            fprintf(stderr, "asprintf failed");
        }
    }

  private:
    char* _value;

    DISALLOW_COPY_AND_ASSIGN(PrintfPrinter);
};

class SignedIntegerPrinter : public UnsignedIntegerPrinter {
  public:
    SignedIntegerPrinter(int64_t value, int base, int min_width)
        : UnsignedIntegerPrinter(((value >= 0) ? value : (-value)), base, min_width),
          _negative(value < 0) { }

    virtual void print_to(String* out) const {
        if (_negative) {
            out->append(1, '-');
        }
        UnsignedIntegerPrinter::print_to(out);
    }

  private:
    bool _negative;

    DISALLOW_COPY_AND_ASSIGN(SignedIntegerPrinter);
};

class FloatingPointPrinter : public PrintfPrinter {
  public:
    FloatingPointPrinter(float value)
        : PrintfPrinter("%f", value) { }

    FloatingPointPrinter(double value)
        : PrintfPrinter("%lf", value) { }

    DISALLOW_COPY_AND_ASSIGN(FloatingPointPrinter);
};

}  // namespace

FormatItem::FormatItem() { }

FormatItem::FormatItem(const char* string)
    : _printer(new EncodedStringPrinter(string, ascii_encoding())) { }
FormatItem::FormatItem(const String& string)
    : _printer(new StringPiecePrinter(string)) { }
FormatItem::FormatItem(const StringKey& string)
    : _printer(new StringPiecePrinter(string)) { }
FormatItem::FormatItem(const StringPiece& string)
    : _printer(new StringPiecePrinter(string)) { }

FormatItem::FormatItem(bool b) : _printer(new BooleanPrinter(b)) { }

FormatItem::FormatItem(char ch) : _printer(new CharacterPrinter(ch)) { }

FormatItem::FormatItem(signed char i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }
FormatItem::FormatItem(signed short i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }
FormatItem::FormatItem(signed int i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }
FormatItem::FormatItem(signed long i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }
FormatItem::FormatItem(signed long long i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }

FormatItem::FormatItem(unsigned char i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }
FormatItem::FormatItem(unsigned short i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }
FormatItem::FormatItem(unsigned int i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }
FormatItem::FormatItem(unsigned long i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }
FormatItem::FormatItem(unsigned long long i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }

FormatItem::FormatItem(float f) : _printer(new FloatingPointPrinter(f)) { }
FormatItem::FormatItem(double d) : _printer(new FloatingPointPrinter(d)) { }

FormatItem::FormatItem(const FormatItem& other) : _printer(other._printer) { }

FormatItem& FormatItem::operator=(const FormatItem& other) {
    _printer = other._printer;
    return *this;
}

FormatItem FormatItem::make(FormatItemPrinter* printer) {
    FormatItem result;
    result._printer = printer;
    return result;
}

void FormatItem::print_to(String* out) const {
    if (_printer.get()) {
        _printer->print_to(out);
    }
}

void format(String* out, const char* fmt, SFZ_FORMAT_ITEMS_DEFINITION) {
    const FormatItem* items[16] = SFZ_FORMAT_ITEMS_ARRAY;
    FormatResult result(fmt, SFZ_FORMAT_ITEM_COUNT, items);
    result.print_to(out);
}

void print(int fd, const char* fmt, SFZ_FORMAT_ITEMS_DEFINITION) {
    const FormatItem* items[SFZ_FORMAT_ITEM_COUNT] = SFZ_FORMAT_ITEMS_ARRAY;
    String decoded;
    FormatResult(fmt, SFZ_FORMAT_ITEM_COUNT, items).print_to(&decoded);
    Bytes encoded(decoded, utf8_encoding());
    write(fd, encoded.data(), encoded.size());
}

FormatResult::FormatResult(const char* fmt, size_t item_count, const FormatItem** items)
    : _fmt(fmt),
      _item_count(item_count),
      _items(items) { }

namespace {

void span_complement(
        const StringPiece& input, const StringPiece& chars,
        StringPiece* span, StringPiece* remainder) {
    foreach (i, range(input.size())) {
        if (chars.find(input.at(i)) != String::kNone) {
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

UnsignedIntegerPrinter::UnsignedIntegerPrinter(uint64_t value, int base, size_t min_width)
    : _value(value),
      _base(base),
      _min_width(min_width) { }

namespace {

const char kDigits[] = "0123456789abcdef";

}  // namespace

void UnsignedIntegerPrinter::print_to(String* out) const {
    uint8_t buffer[64];
    size_t size = 0;
    uint8_t* data = buffer + 64;
    uint64_t value = _value;

    while (value > 0) {
        --data;
        ++size;
        *data = kDigits[value % _base];
        value /= _base;
    }

    if (size < _min_width) {
        out->append(_min_width - size, '0');
    }
    out->append(BytesPiece(data, size), ascii_encoding());
}

}  // namespace sfz
