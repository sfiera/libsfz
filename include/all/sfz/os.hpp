// Copyright (c) 2010-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_OS_HPP_
#define SFZ_OS_HPP_

#include <sys/stat.h>
#include <pn/string>

namespace sfz {

typedef struct stat Stat;

namespace path {

bool exists(pn::string_view path);
bool isdir(pn::string_view path);
bool isfile(pn::string_view path);
bool islink(pn::string_view path);

std::pair<pn::string_view, pn::string_view> splitdrive(pn::string_view path);
std::pair<pn::string_view, pn::string_view> split(pn::string_view path);
pn::string_view                             dirname(pn::string_view path);
pn::string_view                             basename(pn::string_view path);

pn::string joinv(pn::string_view root, std::initializer_list<pn::string_view> segments);

template <typename... string_views>
pn::string join(pn::string_view root, string_views... segments) {
    return joinv(root, {segments...});
}

}  // namespace path

void       chdir(pn::string_view path);
pn::string getcwd();
void       symlink(pn::string_view content, pn::string_view container);

void mkdir(pn::string_view path, mode_t mode);
void mkfifo(pn::string_view path, mode_t mode);
void makedirs(pn::string_view path, mode_t mode);

void unlink(pn::string_view path);
void rmdir(pn::string_view path);
void rmtree(pn::string_view path);

class TemporaryDirectory {
  public:
    TemporaryDirectory(pn::string_view prefix);
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory(TemporaryDirectory&&)      = delete;

    ~TemporaryDirectory();

    const pn::string& path() const;

  private:
    pn::string _path;
};

class TreeWalker;
enum WalkType { WALK_LOGICAL, WALK_PHYSICAL };
void walk(pn::string_view root, WalkType type, const TreeWalker& visitor);

class TreeWalker {
  public:
    virtual ~TreeWalker();

    virtual void pre_directory(pn::string_view name, const Stat& st) const   = 0;
    virtual void cycle_directory(pn::string_view name, const Stat& st) const = 0;
    virtual void post_directory(pn::string_view name, const Stat& st) const  = 0;

    virtual void file(pn::string_view name, const Stat& st) const = 0;

    virtual void symlink(pn::string_view name, const Stat& st) const        = 0;
    virtual void broken_symlink(pn::string_view name, const Stat& st) const = 0;

    virtual void other(pn::string_view name, const Stat& st) const = 0;
};

}  // namespace sfz

#endif  // SFZ_OS_HPP_
