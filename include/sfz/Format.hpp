// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMAT_HPP_
#define SFZ_FORMAT_HPP_

#include "sfz/Bytes.hpp"
#include "sfz/SmartPtr.hpp"
#include "sfz/String.hpp"
#include "sfz/ReferenceCounted.hpp"

namespace sfz {

class FormatItem;
class FormatItemPrinter;

class FormatItem {
  public:
    // Empty formatter.  Prints an empty string.
    FormatItem();

    // String formatters.  Prints the string, as is.
    FormatItem(const char* string);
    FormatItem(const String& string);
    FormatItem(const StringKey& string);
    FormatItem(const StringPiece& string);

    // Boolean formatter.  Prints either "true" or "false".
    FormatItem(bool b);

    // Character formatter.  Prints the character, raw.
    FormatItem(char ch);

    // Integer formatters.  Prints the number in digits.
    FormatItem(signed char i, int base = 10, int min_width = 1);
    FormatItem(signed short i, int base = 10, int min_width = 1);
    FormatItem(signed int i, int base = 10, int min_width = 1);
    FormatItem(signed long i, int base = 10, int min_width = 1);
    FormatItem(signed long long i, int base = 10, int min_width = 1);

    FormatItem(unsigned char i, int base = 10, int min_width = 1);
    FormatItem(unsigned short i, int base = 10, int min_width = 1);
    FormatItem(unsigned int i, int base = 10, int min_width = 1);
    FormatItem(unsigned long i, int base = 10, int min_width = 1);
    FormatItem(unsigned long long i, int base = 10, int min_width = 1);

    // Floating-point formatters.  Prints using the "%f" printf() code family.
    FormatItem(float f);
    FormatItem(double d);

    // Templated formatter.
    template <typename T> FormatItem(const T& object);

    // Copying and assignment.
    FormatItem(const FormatItem& other);
    FormatItem& operator=(const FormatItem& other);

    static FormatItem make(FormatItemPrinter* printer);

    void print_to(String* out) const;

  private:
    RefPtr<const FormatItemPrinter> _printer;

    // ALLOW_COPY_AND_ASSIGN
};

#define SFZ_FORMAT_ITEM_COUNT 16

#define SFZ_FORMAT_ITEMS_DECLARATION \
        const FormatItem& item0 = FormatItem(), const FormatItem& item1 = FormatItem(), \
        const FormatItem& item2 = FormatItem(), const FormatItem& item3 = FormatItem(), \
        const FormatItem& item4 = FormatItem(), const FormatItem& item5 = FormatItem(), \
        const FormatItem& item6 = FormatItem(), const FormatItem& item7 = FormatItem(), \
        const FormatItem& item8 = FormatItem(), const FormatItem& item9 = FormatItem(), \
        const FormatItem& item10 = FormatItem(), const FormatItem& item11 = FormatItem(), \
        const FormatItem& item12 = FormatItem(), const FormatItem& item13 = FormatItem(), \
        const FormatItem& item14 = FormatItem(), const FormatItem& item15 = FormatItem()

#define SFZ_FORMAT_ITEMS_DEFINITION \
        const FormatItem& item0, const FormatItem& item1, \
        const FormatItem& item2, const FormatItem& item3, \
        const FormatItem& item4, const FormatItem& item5, \
        const FormatItem& item6, const FormatItem& item7, \
        const FormatItem& item8, const FormatItem& item9, \
        const FormatItem& item10, const FormatItem& item11, \
        const FormatItem& item12, const FormatItem& item13, \
        const FormatItem& item14, const FormatItem& item15

#define SFZ_FORMAT_ITEMS_CALL \
    item0, item1, item2, item3, item4, item5, item6, item7, \
    item8, item9, item10, item11, item12, item13, item14, item15

#define SFZ_FORMAT_ITEMS_ARRAY { \
    &item0, &item1, &item2, &item3, &item4, &item5, &item6, &item7, \
    &item8, &item9, &item10, &item11, &item12, &item13, &item14, &item15 \
}

void format(String* out, const char* fmt, SFZ_FORMAT_ITEMS_DECLARATION);

class FormatResult {
  public:
    FormatResult(const char* fmt, size_t item_count, const FormatItem** items);

    void print_to(String* out) const;

  private:
    const char* _fmt;
    const size_t _item_count;
    const FormatItem* const* const _items;
};

// Implementation details follow.

class FormatItemPrinter : public ReferenceCounted {
  public:
    virtual void print_to(String* out) const = 0;
};

template <typename T>
class TemplatedFormatItemPrinter : public FormatItemPrinter {
  public:
    TemplatedFormatItemPrinter(const T& object)
        : _object(object) { }

    virtual void print_to(String* out) const {
        _object.print_to(out);
    }

  private:
    const T& _object;

    DISALLOW_COPY_AND_ASSIGN(TemplatedFormatItemPrinter);
};

class UnsignedIntegerPrinter : public FormatItemPrinter {
  public:
    UnsignedIntegerPrinter(uint64_t value, int base, size_t min_width);
    virtual void print_to(String* out) const;

  private:
    uint64_t _value;
    int _base;
    size_t _min_width;

    DISALLOW_COPY_AND_ASSIGN(UnsignedIntegerPrinter);
};

template <typename T>
class TemplatedFormatItemPrinter<T*> : public UnsignedIntegerPrinter {
  public:
    TemplatedFormatItemPrinter(const T* pointer)
        : UnsignedIntegerPrinter(pointer, 16, sizeof(void*) / 4) { }

    DISALLOW_COPY_AND_ASSIGN(TemplatedFormatItemPrinter);
};

template <typename T>
FormatItem::FormatItem(const T& object)
    : _printer(new TemplatedFormatItemPrinter<T>(object)) { }

}  // namespace sfz

#endif  // SFZ_FORMAT_HPP_
