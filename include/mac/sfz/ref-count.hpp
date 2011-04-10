// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_REFERENCE_COUNTED_HPP_
#define SFZ_REFERENCE_COUNTED_HPP_

#include <stdint.h>
#include <algorithm>
#include <sfz/macros.hpp>

namespace sfz {

class ReferenceCounted {
  public:
    ReferenceCounted();
    virtual ~ReferenceCounted();

    void ref() const;
    bool unref() const;

  private:
    mutable int32_t _reference_count;

    DISALLOW_COPY_AND_ASSIGN(ReferenceCounted);
};

}  // namespace sfz

#endif  // SFZ_REFERENCE_COUNTED_HPP_
