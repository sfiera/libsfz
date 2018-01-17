// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
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
#include <sfz/exception.hpp>
#include <sfz/posix-format.hpp>

namespace sfz {

mapped_file::mapped_file(pn::string_view path)
        : _path(path.copy()), _file(pn::open(_path.c_str(), "r")) {
    if (!_file) {
        throw std::runtime_error(pn::format("{0}: {1}", path, posix_strerror().string()).c_str());
    }
    struct stat st;
    if (fstat(fileno(_file.c_obj()), &st) < 0) {
        throw std::runtime_error(pn::format("{0}: {1}", path, posix_strerror().string()).c_str());
    }
    if (S_ISDIR(st.st_mode)) {
        throw std::runtime_error(
                pn::format("{0}: {1}", path, posix_strerror(EISDIR).string()).c_str());
    }
    _size = st.st_size;
    _data = reinterpret_cast<uint8_t*>(
            mmap(NULL, _size, PROT_READ, MAP_PRIVATE, fileno(_file.c_obj()), 0));
    if (_data == MAP_FAILED) {
        throw std::runtime_error(pn::format("{0}: {1}", path, posix_strerror().string()).c_str());
    }

    _file = std::move(_file);
}

mapped_file::~mapped_file() { munmap(_data, _size); }

}  // namespace sfz
