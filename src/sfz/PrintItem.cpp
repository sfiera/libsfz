// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/PrintItem.hpp"

namespace sfz {

namespace {

class EncodedStringPrinter : public PrintItem::Impl {
  public:
    EncodedStringPrinter(const char* string, const Encoding& encoding)
        : _string(string),
          _encoding(encoding) { }

    virtual void print_to(PrintTarget out) const {
        out.append(
                BytesPiece(reinterpret_cast<const uint8_t*>(_string), strlen(_string)), _encoding);
    }

  private:
    const char* const _string;
    const Encoding& _encoding;

    DISALLOW_COPY_AND_ASSIGN(EncodedStringPrinter);
};

class BooleanPrinter : public PrintItem::Impl {
  public:
    BooleanPrinter(bool b)
        : _value(b) { }

    virtual void print_to(PrintTarget out) const {
        if (_value) {
            out.append("true");
        } else {
            out.append("false");
        }
    }

  private:
    bool _value;

    DISALLOW_COPY_AND_ASSIGN(BooleanPrinter);
};

class CharacterPrinter : public PrintItem::Impl {
  public:
    CharacterPrinter(char ch)
        : _value(ch) { }

    virtual void print_to(PrintTarget out) const {
        out.append(1, _value);
    }

  private:
    char _value;

    DISALLOW_COPY_AND_ASSIGN(CharacterPrinter);
};

class PrintfPrinter : public PrintItem::Impl {
  public:
    ~PrintfPrinter() {
        if (_value) {
            free(_value);
        }
    }

    virtual void print_to(PrintTarget out) const {
        out.append(_value);
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

class UnsignedIntegerPrinter : public PrintItem::Impl {
  public:
    UnsignedIntegerPrinter(uint64_t value, int base, size_t min_width);
    virtual void print_to(PrintTarget out) const;

  private:
    uint64_t _value;
    int _base;
    size_t _min_width;

    DISALLOW_COPY_AND_ASSIGN(UnsignedIntegerPrinter);
};

class SignedIntegerPrinter : public UnsignedIntegerPrinter {
  public:
    SignedIntegerPrinter(int64_t value, int base, int min_width)
        : UnsignedIntegerPrinter(((value >= 0) ? value : (-value)), base, min_width),
          _negative(value < 0) { }

    virtual void print_to(PrintTarget out) const {
        if (_negative) {
            out.append(1, '-');
        }
        UnsignedIntegerPrinter::print_to(out);
    }

  private:
    const bool _negative;

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

PrintItem::PrintItem() { }

PrintItem::PrintItem(const char* string)
    : _printer(new EncodedStringPrinter(string, ascii_encoding())) { }

PrintItem::PrintItem(bool b) : _printer(new BooleanPrinter(b)) { }

PrintItem::PrintItem(char ch) : _printer(new CharacterPrinter(ch)) { }

PrintItem::PrintItem(signed char i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }
PrintItem::PrintItem(signed short i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }
PrintItem::PrintItem(signed int i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }
PrintItem::PrintItem(signed long i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }
PrintItem::PrintItem(signed long long i, int base, int min_width)
    : _printer(new SignedIntegerPrinter(i, base, min_width)) { }

PrintItem::PrintItem(unsigned char i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }
PrintItem::PrintItem(unsigned short i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }
PrintItem::PrintItem(unsigned int i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }
PrintItem::PrintItem(unsigned long i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }
PrintItem::PrintItem(unsigned long long i, int base, int min_width)
    : _printer(new UnsignedIntegerPrinter(i, base, min_width)) { }

PrintItem::PrintItem(float f) : _printer(new FloatingPointPrinter(f)) { }
PrintItem::PrintItem(double d) : _printer(new FloatingPointPrinter(d)) { }

PrintItem::PrintItem(const void* pointer)
    : _printer(new UnsignedIntegerPrinter(
                reinterpret_cast<uintptr_t>(pointer), 16, sizeof(const void*) * 2)) { }

PrintItem::Impl::~Impl() { }

PrintItem PrintItem::make(Impl* printer) {
    PrintItem result;
    result._printer = printer;
    return result;
}

void PrintItem::print_to(PrintTarget out) const {
    if (_printer.get()) {
        _printer->print_to(out);
    }
}

UnsignedIntegerPrinter::UnsignedIntegerPrinter(uint64_t value, int base, size_t min_width)
    : _value(value),
      _base(base),
      _min_width(min_width) { }

void UnsignedIntegerPrinter::print_to(PrintTarget out) const {
    static const char kDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

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
        out.append(_min_width - size, '0');
    }
    out.append(BytesPiece(data, size), ascii_encoding());
}

}  // namespace sfz
