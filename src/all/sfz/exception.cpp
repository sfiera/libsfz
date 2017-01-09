// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/exception.hpp>

#include <sfz/string-utils.hpp>

namespace sfz {

Exception::Impl::Impl(const PrintItem& msg) : message(msg), what(message) {}

Exception::Exception(const PrintItem& message) : _impl(new Impl(message)) {}

Exception::~Exception() throw() {}

const String& Exception::message() const throw() { return _impl->message; }

const char* Exception::what() const throw() {
    return reinterpret_cast<const char*>(_impl->what.data());
}

}  // namespace sfz
