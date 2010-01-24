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
#include "sfz/Exception.hpp"
#include "sfz/PosixFormatter.hpp"
#include "sfz/ScopedFd.hpp"

namespace sfz {

MappedFile::MappedFile(const StringPiece& path)
        : _path(path),
          _size(0),
          _data(NULL) {
    Bytes path_bytes(path, utf8_encoding());
    path_bytes.resize(path_bytes.size() + 1);
    const char* c_path = reinterpret_cast<const char*>(path_bytes.data());
    _fd = open(c_path, O_RDONLY, 0600);
    if (_fd < 0) {
        throw Exception("{0}: {1}", path, posix_strerror());
    }
    ScopedFd close(_fd);

    struct stat st;
    if (fstat(_fd, &st) < 0) {
        throw Exception("{0}: {1}", path, posix_strerror());
    }
    if (S_ISDIR(st.st_mode)) {
        throw Exception("{0}: {1}", path, posix_strerror(EISDIR));
    }
    _size = st.st_size;

    _data = reinterpret_cast<uint8_t*>(mmap(NULL, _size, PROT_READ, MAP_PRIVATE, _fd, 0));
    if (_data == MAP_FAILED) {
        throw Exception("{0}: {1}", path, posix_strerror());
    }

    close.release();
}

MappedFile::~MappedFile() {
    munmap(const_cast<uint8_t*>(_data), _size);
    close(_fd);
}

StringPiece MappedFile::path() const {
    return _path;
}

BytesPiece MappedFile::data() const {
    return BytesPiece(_data, _size);
}

}  // namespace sfz
