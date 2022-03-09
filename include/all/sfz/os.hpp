// Copyright (c) 2010-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef SFZ_OS_HPP_
#define SFZ_OS_HPP_

#include <sys/stat.h>
#include <memory>
#include <pn/string>

namespace sfz {

#ifdef _WIN32
typedef struct _stat Stat;
#else
typedef struct stat Stat;
#endif

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

#ifdef _MSC_VER
typedef int mkdir_mode_t;
#else
typedef mode_t mkdir_mode_t;
#endif

void mkdir(pn::string_view path, mkdir_mode_t mode);
void mkfifo(pn::string_view path, mkdir_mode_t mode);
void makedirs(pn::string_view path, mkdir_mode_t mode);

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

class scandir_container {
  public:
    struct entry {
        pn::string name;
        Stat       st;
    };
    class iterator {
      public:
        iterator();
        iterator(pn::string dir);
        iterator(iterator&&);
        ~iterator();

        const entry& operator*() const { return _entry; }
        const entry* operator->() const { return &_entry; }
        bool         operator==(const iterator& other) const { return _state == other._state; }
        bool         operator!=(const iterator& other) const { return _state != other._state; }
        iterator&    operator++();

      private:
        pn::string                             _dir;
        std::unique_ptr<void, void (*)(void*)> _state;
        entry                                  _entry;
    };

    scandir_container(pn::string_view path) : _it{path.copy()} {}
    iterator begin() { return std::move(_it); }
    iterator end() { return iterator{}; }

  private:
    iterator _it;
};
inline scandir_container scandir(pn::string_view path) { return scandir_container{path}; }

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
