// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_OS_HPP_
#define SFZ_OS_HPP_

#include <sys/stat.h>
#include <sfz/macros.hpp>
#include <sfz/string.hpp>

namespace sfz {

typedef struct stat Stat;

namespace path {

bool exists(const StringSlice& path);
bool isdir(const StringSlice& path);
bool isfile(const StringSlice& path);
bool islink(const StringSlice& path);

StringSlice basename(const StringSlice& path);
StringSlice dirname(const StringSlice& path);

}  // namespace path

void chdir(const StringSlice& path);
void symlink(const StringSlice& content, const StringSlice& container);

int open(const StringSlice& path, int oflag, mode_t mode);

void mkdir(const StringSlice& path, mode_t mode);
void mkfifo(const StringSlice& path, mode_t mode);
void makedirs(const StringSlice& path, mode_t mode);

void unlink(const StringSlice& path);
void rmdir(const StringSlice& path);
void rmtree(const StringSlice& path);

class TemporaryDirectory {
  public:
    TemporaryDirectory(const StringSlice& prefix);
    ~TemporaryDirectory();

    const String& path() const;

  private:
    String _path;
    DISALLOW_COPY_AND_ASSIGN(TemporaryDirectory);
};

class TreeWalker;
enum WalkType { WALK_LOGICAL, WALK_PHYSICAL };
void walk(const StringSlice& root, WalkType type, const TreeWalker& visitor);

class TreeWalker {
  public:
    virtual ~TreeWalker();

    virtual void pre_directory(const StringSlice& name, const Stat& st) const   = 0;
    virtual void cycle_directory(const StringSlice& name, const Stat& st) const = 0;
    virtual void post_directory(const StringSlice& name, const Stat& st) const  = 0;

    virtual void file(const StringSlice& name, const Stat& st) const = 0;

    virtual void symlink(const StringSlice& name, const Stat& st) const        = 0;
    virtual void broken_symlink(const StringSlice& name, const Stat& st) const = 0;

    virtual void other(const StringSlice& name, const Stat& st) const = 0;
};

}  // namespace sfz

#endif  // SFZ_OS_HPP_
