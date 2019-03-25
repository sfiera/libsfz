// Copyright (c) 2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_ERROR_HPP_
#define SFZ_ERROR_HPP_

#include <pn/string>

namespace sfz {

int        win_last_error();
pn::string win_strerror(int error = 0);

int        posix_errno();
pn::string posix_strerror(int error = 0);

}  // namespace sfz

#endif  // SFZ_ERROR_HPP_
