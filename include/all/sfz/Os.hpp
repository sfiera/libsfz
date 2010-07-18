// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_OS_HPP_
#define SFZ_OS_HPP_

#include <sys/stat.h>
#include <sfz/String.hpp>

namespace sfz {

typedef struct stat Stat;

namespace path {

bool exists(const StringPiece& path);
bool isdir(const StringPiece& path);
bool isfile(const StringPiece& path);
bool islink(const StringPiece& path);

StringPiece basename(const StringPiece& path);
StringPiece dirname(const StringPiece& path);

}  // namespace path

void chdir(const StringPiece& path);
void symlink(const StringPiece& content, const StringPiece& container);

int open(const StringPiece& path, int oflag, mode_t mode);

void mkdir(const StringPiece& path, mode_t mode);
void mkfifo(const StringPiece& path, mode_t mode);
void makedirs(const StringPiece& path, mode_t mode);

void unlink(const StringPiece& path);
void rmdir(const StringPiece& path);
void rmtree(const StringPiece& path);

class TemporaryDirectory {
  public:
    TemporaryDirectory(const StringPiece& prefix);
    ~TemporaryDirectory();

    const String& path() const;

  private:
    String _path;
    DISALLOW_COPY_AND_ASSIGN(TemporaryDirectory);
};

class TreeWalker;
enum WalkType { WALK_LOGICAL, WALK_PHYSICAL };
void walk(const StringPiece& root, WalkType type, TreeWalker* visitor);

class TreeWalker {
  public:
    virtual ~TreeWalker();

    virtual void pre_directory(const StringPiece& name, const Stat& st) = 0;
    virtual void cycle_directory(const StringPiece& name, const Stat& st) = 0;
    virtual void post_directory(const StringPiece& name, const Stat& st) = 0;

    virtual void file(const StringPiece& name, const Stat& st) = 0;

    virtual void symlink(const StringPiece& name, const Stat& st) = 0;
    virtual void broken_symlink(const StringPiece& name, const Stat& st) = 0;

    virtual void other(const StringPiece& name, const Stat& st) = 0;
};

}  // namespace sfz

#endif  // SFZ_OS_HPP_
