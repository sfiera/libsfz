// Copyright (c) 2009-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/file.hpp>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pn/output>
#include <stdexcept>

#include "error.hpp"

namespace sfz {

mapped_file::mapped_file(pn::string_view path) : _path(path.copy()), _fd(_path) {
    struct stat st;
    if (fstat(_fd.no, &st) < 0) {
        throw std::runtime_error(pn::format("{0}: {1}", path, posix_strerror()).c_str());
    }
    if (S_ISDIR(st.st_mode)) {
        throw std::runtime_error(pn::format("{0}: {1}", path, posix_strerror(EISDIR)).c_str());
    }
    _size = st.st_size;
    _data = reinterpret_cast<uint8_t*>(mmap(NULL, _size, PROT_READ, MAP_PRIVATE, _fd.no, 0));
    if (_data == MAP_FAILED) {
        throw std::runtime_error(pn::format("{0}: {1}", path, posix_strerror()).c_str());
    }
}

mapped_file::~mapped_file() { munmap(_data, _size); }

mapped_file::fd::fd(const pn::string& path) : no{::open(path.c_str(), O_RDONLY)} {
    if (no < 0) {
        throw std::runtime_error(pn::format("{0}: {1}", path, posix_strerror()).c_str());
    }
}

mapped_file::fd::~fd() {
    if (no >= 0) {
        close(no);
    }
}

}  // namespace sfz
