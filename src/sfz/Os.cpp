// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Os.hpp"

#include <fcntl.h>
#include <fts.h>
#include "sfz/Encoding.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Format.hpp"
#include "sfz/PosixFormatter.hpp"

namespace sfz {

namespace path {

bool exists(const StringPiece& path) {
    CString c_str(path);
    Stat st;
    return (stat(c_str.data(), &st) == 0);
}

bool isdir(const StringPiece& path) {
    CString c_str(path);
    Stat st;
    return (stat(c_str.data(), &st) == 0) && ((st.st_mode & S_IFMT) == S_IFDIR);
}

bool isfile(const StringPiece& path) {
    CString c_str(path);
    Stat st;
    return (stat(c_str.data(), &st) == 0) && ((st.st_mode & S_IFMT) == S_IFREG);
}

bool islink(const StringPiece& path) {
    CString c_str(path);
    Stat st;
    return (lstat(c_str.data(), &st) == 0) && ((st.st_mode & S_IFMT) == S_IFLNK);
}

StringPiece basename(const StringPiece& path) {
    if (path == "/") {
        return path;
    }
    size_t pos = path.rfind('/', path.size() - 1);
    if (pos == StringPiece::npos) {
        return path;
    } else if (pos == path.size() - 1) {
        return basename(path.substr(0, path.size() - 1));
    } else {
        return path.substr(pos + 1);
    }
}

StringPiece dirname(const StringPiece& path) {
    size_t pos = path.rfind('/', path.size() - 1);
    if (pos == 0) {
        return "/";
    } else if (pos == StringPiece::npos) {
        return ".";
    } else if (pos == path.size() - 1) {
        return dirname(path.substr(0, path.size() - 1));
    } else {
        return path.substr(0, pos);
    }
}

}  // namespace path

void chdir(const StringPiece& path) {
    CString c_str(path);
    if (::chdir(c_str.data()) < 0) {
        throw Exception(format("chdir: {0}", posix_strerror()));
    }
}

void symlink(const StringPiece& content, const StringPiece& container) {
    CString content_c_str(content);
    CString container_c_str(container);
    if (::symlink(content_c_str.data(), container_c_str.data()) < 0) {
        throw Exception(format("symlink: {0}", posix_strerror()));
    }
}

int open(const StringPiece& path, int oflag, mode_t mode) {
    CString c_str(path);
    int fd = ::open(c_str.data(), oflag, mode);
    if (fd < 0) {
        throw Exception(format("open: {0}", posix_strerror()));
    }
    return fd;
}

void mkdir(const StringPiece& path, mode_t mode) {
    CString c_str(path);
    if (::mkdir(c_str.data(), mode) != 0) {
        throw Exception(format("mkdir: {0}", posix_strerror()));
    }
}

void mkfifo(const StringPiece& path, mode_t mode) {
    CString c_str(path);
    if (::mkfifo(c_str.data(), mode) != 0) {
        throw Exception(format("mkfifo: {0}", posix_strerror()));
    }
}

void makedirs(const StringPiece& path, mode_t mode) {
    if (!path::isdir(path)) {
        makedirs(path::dirname(path), mode);
        mkdir(path, mode);
    }
}

void unlink(const StringPiece& path) {
    CString c_str(path);
    if (::unlink(c_str.data()) < 0) {
        throw Exception(format("unlink: {0}", posix_strerror()));
    }
}

void rmdir(const StringPiece& path) {
    CString c_str(path);
    if (::rmdir(c_str.data()) < 0) {
        throw Exception(format("rmdir: {0}", posix_strerror()));
    }
}

void rmtree(const StringPiece& path) {
    if (path::exists(path)) {
        class RmtreeVisitor : public TreeWalker {
          public:
            void pre_directory(    const StringPiece& path, const Stat&) { }
            void cycle_directory(  const StringPiece& path, const Stat&) { }

            void post_directory(   const StringPiece& path, const Stat&) { rmdir(path); }

            void file(             const StringPiece& path, const Stat&) { unlink(path); }
            void symlink(          const StringPiece& path, const Stat&) { unlink(path); }
            void broken_symlink(   const StringPiece& path, const Stat&) { unlink(path); }
            void other(            const StringPiece& path, const Stat&) { unlink(path); }
        };
        RmtreeVisitor visitor;
        walk(path, WALK_PHYSICAL, &visitor);
    }
}

TemporaryDirectory::TemporaryDirectory(const StringPiece& prefix) {
    String str(format("/tmp/{0}XXXXXX", prefix));
    CString c_str(str);
    mkdtemp(c_str.mutable_data());
    _path.assign(utf8::decode(c_str.data()));
}

TemporaryDirectory::~TemporaryDirectory() {
    rmtree(_path);
}

const String& TemporaryDirectory::path() const {
    return _path;
}

namespace {

class FtsCloser {
  public:
    FtsCloser(FTS* fts) : _fts(fts) { }
    ~FtsCloser() { fts_close(_fts); }
  private:
    FTS* _fts;
    DISALLOW_COPY_AND_ASSIGN(FtsCloser);
};

int compare_ftsent(const FTSENT** lhs, const FTSENT** rhs) {
    return strcmp((*lhs)->fts_name, (*rhs)->fts_name);
}

}  // namespace

void walk(const StringPiece& root, WalkType type, TreeWalker* visitor) {
    CString c_str(root);
    char* const pathv[] = { c_str.mutable_data(), NULL };
    int options = FTS_NOCHDIR;
    if (type == WALK_PHYSICAL) {
        options |= FTS_PHYSICAL;
    } else {
        options |= FTS_LOGICAL;
    }

    FTS* fts = fts_open(pathv, options, compare_ftsent);
    if (fts == NULL) {
        throw Exception(format("fts_open: {0}", posix_strerror()));
    }
    FtsCloser deleter(fts);
    while (FTSENT* ent = fts_read(fts)) {
        String path(utf8::decode(ent->fts_path));
        const Stat& st = *ent->fts_statp;
        switch (ent->fts_info) {
          case FTS_D:        visitor->pre_directory(    path, st); break;
          case FTS_DC:       visitor->cycle_directory(  path, st); break;
          case FTS_DEFAULT:  visitor->other(            path, st); break;
          case FTS_DP:       visitor->post_directory(   path, st); break;
          case FTS_F:        visitor->file(             path, st); break;
          case FTS_SL:       visitor->symlink(          path, st); break;
          case FTS_SLNONE:   visitor->broken_symlink(   path, st); break;

          case FTS_DNR:
          case FTS_ERR:
          case FTS_NS:
            throw Exception(format("fts_read: {0}", posix_strerror(ent->fts_errno)));

          case FTS_DOT:
          case FTS_NSOK:
          default:
            throw Exception(format("walk: got invalid type {0}", ent->fts_info));
        }
    }
}

TreeWalker::~TreeWalker() { }

}  // namespace sfz
