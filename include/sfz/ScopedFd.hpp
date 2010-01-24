// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_SCOPED_FD_HPP_
#define SFZ_SCOPED_FD_HPP_

namespace sfz {

class ScopedFd {
  public:
    ScopedFd(int fd);
    ~ScopedFd();

    int get();
    int release();

  private:
    int _fd;
};

}  // namespace sfz

#endif // SFZ_SCOPED_FD_HPP_
