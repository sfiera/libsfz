// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/memory.hpp>

#include <sfz/exception.hpp>

namespace sfz {

linked_group::linked_group() : _prev(this), _next(this) { }

void linked_group::join(linked_group* group) {
    _next = group->_next;
    _next->_prev = this;
    _prev = group;
    _prev->_next = this;
}

bool linked_group::depart() {
    if (unique()) {
        return true;
    }
    _prev->_next = _next;
    _next->_prev = _prev;
    _prev = _next = this;
    return false;
}

void linked_group::check_unique() const {
    if (!unique()) {
        throw Exception("linked_ptr<> is not unique()");
    }
}

bool linked_group::unique() const {
    return _next == this;
}

}  // namespace sfz
