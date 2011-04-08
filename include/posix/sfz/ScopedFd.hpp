// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_SCOPED_FD_HPP_
#define SFZ_SCOPED_FD_HPP_

#include <sfz/bytes.hpp>
#include <sfz/macros.hpp>

namespace sfz {

class ScopedFd {
  public:
    explicit ScopedFd(int fd);
    ~ScopedFd();

    int get();
    int release();

    void push(const BytesSlice& bytes);
    void push(size_t num, uint8_t byte);

  private:
    int _fd;

    DISALLOW_COPY_AND_ASSIGN(ScopedFd);
};

}  // namespace sfz

#endif // SFZ_SCOPED_FD_HPP_
