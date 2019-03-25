// Copyright (c) 2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/os.hpp>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>
#include <pn/output>
#include <sfz/encoding.hpp>
#include <sfz/error.hpp>
#include <stdexcept>

namespace sfz {

namespace path {

static int check_file_attributes(pn::string_view path, DWORD mask) {
    DWORD attributes = GetFileAttributesA(path.copy().c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (attributes & mask) == mask;
}

bool exists(pn::string_view path) { return check_file_attributes(path, 0); }

bool isdir(pn::string_view path) { return check_file_attributes(path, FILE_ATTRIBUTE_DIRECTORY); }

bool isfile(pn::string_view path) { return check_file_attributes(path, FILE_ATTRIBUTE_NORMAL); }

bool islink(pn::string_view path) {
    return check_file_attributes(path, FILE_ATTRIBUTE_REPARSE_POINT);
}

pn::string_view basename(pn::string_view path) {
    if (path == "/") {
        return path;
    }
    int pos = path.rfind(pn::rune{'/'});
    if (pos == path.npos) {
        return path;
    } else if (pos == path.size() - 1) {
        return basename(path.substr(0, path.size() - 1));
    } else {
        return path.substr(pos + 1);
    }
}

pn::string_view dirname(pn::string_view path) {
    int pos = path.rfind(pn::rune{'/'});
    if (pos == 0) {
        return "/";
    } else if (pos == path.npos) {
        return ".";
    } else if (pos == path.size() - 1) {
        return dirname(path.substr(0, path.size() - 1));
    } else {
        return path.substr(0, pos);
    }
}

}  // namespace path

void chdir(pn::string_view path) {
    if (!SetCurrentDirectory(path.copy().c_str())) {
        throw std::runtime_error(pn::format("chdir: {0}: {1}", path, posix_strerror()).c_str());
    }
}

void symlink(pn::string_view content, pn::string_view container) {
    static_cast<void>(content);
    throw std::runtime_error(pn::format("symlink: {0}: not supported", container).c_str());
}

void mkdir(pn::string_view path, mode_t mode) {
    static_cast<void>(mode);
    if (!CreateDirectoryA(path.copy().c_str(), nullptr)) {
        throw std::runtime_error(pn::format("mkdir: {0}: {1}", path, posix_strerror()).c_str());
    }
}

void mkfifo(pn::string_view path, mode_t mode) {
    static_cast<void>(mode);
    throw std::runtime_error(pn::format("mkfifo: {0}: not supported", path).c_str());
}

void makedirs(pn::string_view path, mode_t mode) {
    if (!path::isdir(path)) {
        makedirs(path::dirname(path), mode);
        mkdir(path, mode);
    }
}

void unlink(pn::string_view path) {
    if (::unlink(path.copy().c_str()) < 0) {
        throw std::runtime_error(pn::format("unlink: {0}: {1}", path, posix_strerror()).c_str());
    }
}

void rmdir(pn::string_view path) {
    if (::rmdir(path.copy().c_str()) < 0) {
        throw std::runtime_error(pn::format("rmdir: {0}: {1}", path, posix_strerror()).c_str());
    }
}

void rmtree(pn::string_view path) {
    if (path::exists(path)) {
        class RmtreeVisitor : public TreeWalker {
          public:
            void pre_directory(pn::string_view path, const Stat& stat) const {
                static_cast<void>(path);
                static_cast<void>(stat);
            }
            void cycle_directory(pn::string_view path, const Stat& stat) const {
                static_cast<void>(path);
                static_cast<void>(stat);
            }

            void post_directory(pn::string_view path, const Stat&) const { rmdir(path); }

            void file(pn::string_view path, const Stat&) const { unlink(path); }
            void symlink(pn::string_view path, const Stat&) const { unlink(path); }
            void broken_symlink(pn::string_view path, const Stat&) const { unlink(path); }
            void other(pn::string_view path, const Stat&) const { unlink(path); }
        };
        RmtreeVisitor visitor;
        walk(path, WALK_PHYSICAL, visitor);
    }
}

TemporaryDirectory::TemporaryDirectory(pn::string_view prefix) {
    char temp_path_buf[MAX_PATH + 1];
    if (GetTempPathA(MAX_PATH + 1, temp_path_buf) == 0) {
        throw std::runtime_error(pn::format("GetTempPath: {0}", win_strerror()).c_str());
    }
    char temp_file_buf[MAX_PATH + 1];
    if (GetTempFileNameA(temp_path_buf, prefix.copy().c_str(), 0, temp_file_buf) == 0) {
        throw std::runtime_error(pn::format("GetTempFileName: {0}", win_strerror()).c_str());
    }
    pn::string temp_file = temp_file_buf;
    unlink(temp_file);
    mkdir(temp_file, 0000);
    _path = std::move(temp_file);
}

TemporaryDirectory::~TemporaryDirectory() { rmtree(_path); }

const pn::string& TemporaryDirectory::path() const { return _path; }

void walk(pn::string_view root, WalkType type, const TreeWalker& visitor) {
    static_cast<void>(root);
    static_cast<void>(type);
    static_cast<void>(visitor);
    throw std::runtime_error("walk: not supported");
}

TreeWalker::~TreeWalker() {}

}  // namespace sfz
