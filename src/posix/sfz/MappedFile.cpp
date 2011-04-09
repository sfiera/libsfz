// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/MappedFile.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "sfz/PosixFormatter.hpp"
#include "sfz/ScopedFd.hpp"
#include "sfz/exception.hpp"
#include "sfz/format.hpp"
#include "sfz/string-utils.hpp"

namespace sfz {

MappedFile::MappedFile(const StringSlice& path)
        : _path(path),
          _size(0),
          _data(NULL) {
    CString c_path(path);
    _fd = open(c_path.data(), O_RDONLY, 0600);
    if (_fd < 0) {
        throw Exception(format("{0}: {1}", path, posix_strerror()));
    }
    ScopedFd close(_fd);

    struct stat st;
    if (fstat(_fd, &st) < 0) {
        throw Exception(format("{0}: {1}", path, posix_strerror()));
    }
    if (S_ISDIR(st.st_mode)) {
        throw Exception(format("{0}: {1}", path, posix_strerror(EISDIR)));
    }
    _size = st.st_size;

    _data = reinterpret_cast<uint8_t*>(mmap(NULL, _size, PROT_READ, MAP_PRIVATE, _fd, 0));
    if (_data == MAP_FAILED) {
        throw Exception(format("{0}: {1}", path, posix_strerror()));
    }

    close.release();
}

MappedFile::~MappedFile() {
    munmap(const_cast<uint8_t*>(_data), _size);
    close(_fd);
}

StringSlice MappedFile::path() const {
    return _path;
}

BytesSlice MappedFile::data() const {
    return BytesSlice(_data, _size);
}

}  // namespace sfz
