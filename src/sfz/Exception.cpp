// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Exception.hpp"

#include "sfz/Encoding.hpp"
#include "sfz/Format.hpp"

namespace sfz {

Exception::Exception(const PrintItem& message) {
    _message.append(message);
    _utf8_string.assign(utf8::encode(_message));
    _utf8_string.resize(_utf8_string.size() + 1);
}

Exception::~Exception() throw() { }

const String& Exception::message() const throw() {
    return _message;
}

const char* Exception::what() const throw() {
    return reinterpret_cast<const char*>(_utf8_string.data());
}

}  // namespace sfz
