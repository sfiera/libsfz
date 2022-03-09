// Copyright (c) 2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "win.hpp"

#include <sfz/file.hpp>

#include <errno.h>
#include <fcntl.h>
#include <memoryapi.h>
#include <stdio.h>
#include <pn/output>
#include <sfz/error.hpp>
#include <stdexcept>

namespace sfz {

static LONGLONG file_size(pn::string_view path, HANDLE h) {
    LARGE_INTEGER size;
    if (!GetFileSizeEx(h, &size)) {
        throw std::runtime_error(pn::format("{0}: {1}", path, win_strerror()).c_str());
    }
    return size.QuadPart;
}

mapped_file::mapped_file(pn::string_view path)
        : _path{path.copy()},
          _file{_path, CreateFileW(
                               _path.cpp_wstr().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, 0, nullptr)},
          _size{file_size(_path, _file.h)},
          _file_mapping{_path, CreateFileMappingW(_file.h, NULL, PAGE_READONLY, 0, _size, NULL)},
          _view_of_file{_path, MapViewOfFile(_file_mapping.h, FILE_MAP_READ, 0, 0, _size)} {}

mapped_file::~mapped_file() {}

pn::data_view mapped_file::data() const {
    return pn::data_view{
            reinterpret_cast<const uint8_t*>(_view_of_file.ptr), static_cast<int>(_size)};
}

mapped_file::handle::handle(pn::string_view path, HANDLE handle) : h{handle} {
    if (h == INVALID_HANDLE_VALUE) {
        throw std::runtime_error(pn::format("{0}: {1}", path, win_strerror()).c_str());
    }
}

mapped_file::handle::~handle() {
    if (h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);
    }
}

mapped_file::view_of_file::view_of_file(pn::string_view path, LPVOID p) : ptr{p} {
    if (ptr == NULL) {
        throw std::runtime_error(pn::format("{0}: {1}", path, win_strerror()).c_str());
    }
}

mapped_file::view_of_file::~view_of_file() {
    if (ptr != NULL) {
        UnmapViewOfFile(ptr);
    }
}

}  // namespace sfz
