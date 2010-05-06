// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_FORMAT_HPP_
#define SFZ_FORMAT_HPP_

#include "sfz/Bytes.hpp"
#include "sfz/PrintItem.hpp"
#include "sfz/SmartPtr.hpp"
#include "sfz/String.hpp"
#include "sfz/ReferenceCounted.hpp"

namespace sfz {

#define SFZ_FORMAT_ITEM_COUNT 16

#define SFZ_FORMAT_ITEMS_DECLARATION \
        const PrintItem& item0 = PrintItem(), const PrintItem& item1 = PrintItem(), \
        const PrintItem& item2 = PrintItem(), const PrintItem& item3 = PrintItem(), \
        const PrintItem& item4 = PrintItem(), const PrintItem& item5 = PrintItem(), \
        const PrintItem& item6 = PrintItem(), const PrintItem& item7 = PrintItem(), \
        const PrintItem& item8 = PrintItem(), const PrintItem& item9 = PrintItem(), \
        const PrintItem& item10 = PrintItem(), const PrintItem& item11 = PrintItem(), \
        const PrintItem& item12 = PrintItem(), const PrintItem& item13 = PrintItem(), \
        const PrintItem& item14 = PrintItem(), const PrintItem& item15 = PrintItem()

#define SFZ_FORMAT_ITEMS_DEFINITION \
        const PrintItem& item0, const PrintItem& item1, \
        const PrintItem& item2, const PrintItem& item3, \
        const PrintItem& item4, const PrintItem& item5, \
        const PrintItem& item6, const PrintItem& item7, \
        const PrintItem& item8, const PrintItem& item9, \
        const PrintItem& item10, const PrintItem& item11, \
        const PrintItem& item12, const PrintItem& item13, \
        const PrintItem& item14, const PrintItem& item15

#define SFZ_FORMAT_ITEMS_CALL \
    item0, item1, item2, item3, item4, item5, item6, item7, \
    item8, item9, item10, item11, item12, item13, item14, item15

#define SFZ_FORMAT_ITEMS_ARRAY { \
    &item0, &item1, &item2, &item3, &item4, &item5, &item6, &item7, \
    &item8, &item9, &item10, &item11, &item12, &item13, &item14, &item15 \
}

void format(PrintTarget out, const char* fmt, SFZ_FORMAT_ITEMS_DECLARATION);

void print(int fd, const char* fmt, SFZ_FORMAT_ITEMS_DECLARATION);

class FormatResult {
  public:
    FormatResult(const char* fmt, size_t item_count, const PrintItem** items);

    void print_to(PrintTarget out) const;

  private:
    const char* _fmt;
    const size_t _item_count;
    const PrintItem* const* const _items;
};

}  // namespace sfz

#endif  // SFZ_FORMAT_HPP_
