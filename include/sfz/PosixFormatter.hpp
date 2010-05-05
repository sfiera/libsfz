// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_POSIX_FORMATTER_HPP_
#define SFZ_POSIX_FORMATTER_HPP_

#include "PrintItem.hpp"

namespace sfz {

PrintItem posix_errno();
PrintItem posix_strerror(int error = 0);

}  // namespace sfz

#endif // SFZ_POSIX_FORMATTER_HPP_
