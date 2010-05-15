// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_EXCEPTION_HPP_
#define SFZ_EXCEPTION_HPP_

#include "sfz/Format.hpp"
#include "sfz/String.hpp"

namespace sfz {

class Exception : public std::exception {
  public:
    Exception(const char* fmt, SFZ_FORMAT_ITEMS_DECLARATION);
    virtual ~Exception() throw();
    virtual const String& message() const throw();
    virtual const char* what() const throw();

  private:
    String _message;
    Bytes _utf8_string;
};

}  // namespace sfz

#endif // SFZ_EXCEPTION_HPP_
