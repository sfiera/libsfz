// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_EXCEPTION_HPP_
#define SFZ_EXCEPTION_HPP_

#include <exception>
#include <sfz/memory.hpp>
#include <sfz/print.hpp>
#include <sfz/ref-count.hpp>
#include <sfz/string.hpp>
#include <sfz/string-utils.hpp>

namespace sfz {

class Exception : public std::exception {
  public:
    Exception(const PrintItem& message);
    virtual ~Exception() throw();
    virtual const String& message() const throw();
    virtual const char* what() const throw();

  private:
    struct Impl : public ReferenceCounted {
        Impl(const PrintItem& msg);
        String message;
        CString what;
    };

    scoped_ref<const Impl> _impl;
};

}  // namespace sfz

#endif // SFZ_EXCEPTION_HPP_