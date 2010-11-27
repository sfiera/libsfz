// Copyright (c) 2010 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include "sfz/Os.hpp"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::_;
using testing::Eq;
using testing::InSequence;
using testing::Test;

namespace sfz {
namespace {

typedef Test OsTest;

// Test path::basename() with many different inputs.
TEST_F(OsTest, Basename) {
    EXPECT_THAT(path::basename("/"),                Eq("/"));
    EXPECT_THAT(path::basename("/aesc"),            Eq("aesc"));
    EXPECT_THAT(path::basename("/aesc/"),           Eq("aesc"));
    EXPECT_THAT(path::basename("/aesc/wynn"),       Eq("wynn"));
    EXPECT_THAT(path::basename("/aesc/wynn/"),      Eq("wynn"));
    EXPECT_THAT(path::basename("/aesc/wynn/."),     Eq("."));
    EXPECT_THAT(path::basename("/aesc/wynn/.."),    Eq(".."));

    EXPECT_THAT(path::basename(""),                 Eq(""));
    EXPECT_THAT(path::basename("aesc"),             Eq("aesc"));
    EXPECT_THAT(path::basename("aesc/"),            Eq("aesc"));
    EXPECT_THAT(path::basename("aesc/wynn"),        Eq("wynn"));
    EXPECT_THAT(path::basename("aesc/wynn/"),       Eq("wynn"));
    EXPECT_THAT(path::basename("aesc/wynn/."),      Eq("."));
    EXPECT_THAT(path::basename("aesc/wynn/.."),     Eq(".."));
}

// Test path::basename() with many different inputs.
TEST_F(OsTest, Dirname) {
    EXPECT_THAT(path::dirname("/"),                Eq("/"));
    EXPECT_THAT(path::dirname("/aesc"),            Eq("/"));
    EXPECT_THAT(path::dirname("/aesc/"),           Eq("/"));
    EXPECT_THAT(path::dirname("/aesc/wynn"),       Eq("/aesc"));
    EXPECT_THAT(path::dirname("/aesc/wynn/"),      Eq("/aesc"));
    EXPECT_THAT(path::dirname("/aesc/wynn/."),     Eq("/aesc/wynn"));
    EXPECT_THAT(path::dirname("/aesc/wynn/.."),    Eq("/aesc/wynn"));

    EXPECT_THAT(path::dirname(""),                 Eq("."));
    EXPECT_THAT(path::dirname("aesc"),             Eq("."));
    EXPECT_THAT(path::dirname("aesc/"),            Eq("."));
    EXPECT_THAT(path::dirname("aesc/wynn"),        Eq("aesc"));
    EXPECT_THAT(path::dirname("aesc/wynn/"),       Eq("aesc"));
    EXPECT_THAT(path::dirname("aesc/wynn/."),      Eq("aesc/wynn"));
    EXPECT_THAT(path::dirname("aesc/wynn/.."),     Eq("aesc/wynn"));
}

class MockTreeWalker : public TreeWalker {
  public:
    MOCK_METHOD2(pre_directory, void(const StringSlice& name, const Stat& st));
    MOCK_METHOD2(cycle_directory, void(const StringSlice& name, const Stat& st));
    MOCK_METHOD2(post_directory, void(const StringSlice& name, const Stat& st));
    MOCK_METHOD2(file, void(const StringSlice& name, const Stat& st));
    MOCK_METHOD2(symlink, void(const StringSlice& name, const Stat& st));
    MOCK_METHOD2(broken_symlink, void(const StringSlice& name, const Stat& st));
    MOCK_METHOD2(other, void(const StringSlice& name, const Stat& st));
};

MATCHER(IsDirStat, "") { return (arg.st_mode & S_IFMT) == S_IFDIR; }
MATCHER(IsFileStat, "") { return (arg.st_mode & S_IFMT) == S_IFREG; }
MATCHER(IsLinkStat, "") { return (arg.st_mode & S_IFMT) == S_IFLNK; }
MATCHER(IsFifoStat, "") { return (arg.st_mode & S_IFMT) == S_IFIFO; }

// Create a temporary directory with TemporaryDirectory, then fill it with several directories,
// files, and symlinks.  Try walking the directory with a few different options; then, delete some
// of the files and directories and try walking again.
//
// This exercises most of Os.hpp, aside from the simple functions tested above, and some of the
// stranger cases during walk().
TEST_F(OsTest, Hierarchy) {
    TemporaryDirectory dir("os-test");

    ASSERT_THAT(path::exists(dir.path()), Eq(true));
    ASSERT_THAT(path::isdir(dir.path()), Eq(true));
    ASSERT_THAT(path::isfile(dir.path()), Eq(false));

    chdir(dir.path());
    makedirs("./roman/upper", 0700);
    close(open("./roman/upper/A", O_WRONLY | O_CREAT | O_EXCL, 0600));
    close(open("./roman/upper/B", O_WRONLY | O_CREAT | O_EXCL, 0600));
    close(open("./roman/upper/Z", O_WRONLY | O_CREAT | O_EXCL, 0600));
    makedirs("./roman/lower", 0700);
    close(open("./roman/README", O_WRONLY | O_CREAT | O_EXCL, 0600));
    makedirs("./cyrillic/upper", 0700);
    symlink("../../roman/upper/A", "./cyrillic/upper/A");
    makedirs("./cyrillic/lower", 0700);
    symlink("../../roman/lower/a", "./cyrillic/lower/a");
    close(open("./cyrillic/README", O_WRONLY | O_CREAT | O_EXCL, 0600));

    EXPECT_THAT(path::exists("./roman/upper"), Eq(true));
    EXPECT_THAT(path::isdir("./roman/upper"), Eq(true));
    EXPECT_THAT(path::isfile("./roman/upper"), Eq(false));
    EXPECT_THAT(path::islink("./roman/upper"), Eq(false));

    EXPECT_THAT(path::exists("./roman/upper/A"), Eq(true));
    EXPECT_THAT(path::isdir("./roman/upper/A"), Eq(false));
    EXPECT_THAT(path::isfile("./roman/upper/A"), Eq(true));
    EXPECT_THAT(path::islink("./roman/upper/A"), Eq(false));

    EXPECT_THAT(path::exists("./cyrillic/upper/A"), Eq(true));
    EXPECT_THAT(path::isdir("./cyrillic/upper/A"), Eq(false));
    EXPECT_THAT(path::isfile("./cyrillic/upper/A"), Eq(true));
    EXPECT_THAT(path::islink("./cyrillic/upper/A"), Eq(true));

    EXPECT_THAT(path::exists("./cyrillic/lower/a"), Eq(false));
    EXPECT_THAT(path::isdir("./cyrillic/lower/a"), Eq(false));
    EXPECT_THAT(path::isfile("./cyrillic/lower/a"), Eq(false));
    EXPECT_THAT(path::islink("./cyrillic/lower/a"), Eq(true));

    EXPECT_THAT(path::exists("./cyrillic/lower/Z"), Eq(false));
    EXPECT_THAT(path::isdir("./cyrillic/lower/Z"), Eq(false));
    EXPECT_THAT(path::isfile("./cyrillic/lower/Z"), Eq(false));
    EXPECT_THAT(path::islink("./cyrillic/lower/Z"), Eq(false));

    {
        MockTreeWalker walker;
        InSequence s;
        EXPECT_CALL(walker, pre_directory(StringSlice("roman"), IsDirStat()));
        EXPECT_CALL(walker, file(StringSlice("roman/README"), IsFileStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("roman/lower"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("roman/lower"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("roman/upper"), IsDirStat()));
        EXPECT_CALL(walker, file(StringSlice("roman/upper/A"), IsFileStat()));
        EXPECT_CALL(walker, file(StringSlice("roman/upper/B"), IsFileStat()));
        EXPECT_CALL(walker, file(StringSlice("roman/upper/Z"), IsFileStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("roman/upper"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("roman"), IsDirStat()));
        walk("roman", WALK_PHYSICAL, &walker);
    }

    {
        MockTreeWalker walker;
        InSequence s;
        EXPECT_CALL(walker, pre_directory(StringSlice("cyrillic"), IsDirStat()));
        EXPECT_CALL(walker, file(StringSlice("cyrillic/README"), IsFileStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("cyrillic/lower"), IsDirStat()));
        EXPECT_CALL(walker, symlink(StringSlice("cyrillic/lower/a"), IsLinkStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("cyrillic/lower"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, symlink(StringSlice("cyrillic/upper/A"), IsLinkStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("cyrillic"), IsDirStat()));
        walk("cyrillic", WALK_PHYSICAL, &walker);
    }

    {
        MockTreeWalker walker;
        InSequence s;
        EXPECT_CALL(walker, pre_directory(StringSlice("cyrillic"), IsDirStat()));
        EXPECT_CALL(walker, file(StringSlice("cyrillic/README"), IsFileStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("cyrillic/lower"), IsDirStat()));
        EXPECT_CALL(walker, broken_symlink(StringSlice("cyrillic/lower/a"), IsLinkStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("cyrillic/lower"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, file(StringSlice("cyrillic/upper/A"), IsFileStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("cyrillic"), IsDirStat()));
        walk("cyrillic", WALK_LOGICAL, &walker);
    }

    unlink("cyrillic/lower/a");
    unlink("cyrillic/README");
    unlink("roman/README");
    rmdir("cyrillic/lower");
    rmtree("roman/upper");

    {
        MockTreeWalker walker;
        InSequence s;
        EXPECT_CALL(walker, pre_directory(StringSlice("."), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("./cyrillic"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("./cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, broken_symlink(StringSlice("./cyrillic/upper/A"), IsLinkStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("./cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("./cyrillic"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("./roman"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(StringSlice("./roman/lower"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("./roman/lower"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("./roman"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(StringSlice("."), IsDirStat()));
        walk(".", WALK_LOGICAL, &walker);
    }
}

// Exercise the remaining bits of Os.hpp: mkfifo, as well as walk() calls which include files of
// type other() and cycle_directory().
TEST_F(OsTest, WalkOther) {
    TemporaryDirectory dir("os-test");

    ASSERT_THAT(path::exists(dir.path()), Eq(true));
    ASSERT_THAT(path::isdir(dir.path()), Eq(true));
    ASSERT_THAT(path::isfile(dir.path()), Eq(false));

    chdir(dir.path());
    makedirs("./aesc/wynn", 0700);
    symlink("../..", "./aesc/wynn/eth");
    mkfifo("./aesc/thorn", 0600);

    MockTreeWalker walker;
    InSequence s;
    EXPECT_CALL(walker, pre_directory(StringSlice("."), IsDirStat()));
    EXPECT_CALL(walker, pre_directory(StringSlice("./aesc"), IsDirStat()));
    EXPECT_CALL(walker, other(StringSlice("./aesc/thorn"), IsFifoStat()));
    EXPECT_CALL(walker, pre_directory(StringSlice("./aesc/wynn"), IsDirStat()));
    EXPECT_CALL(walker, cycle_directory(StringSlice("./aesc/wynn/eth"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(StringSlice("./aesc/wynn"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(StringSlice("./aesc"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(StringSlice("."), IsDirStat()));
    walk(".", WALK_LOGICAL, &walker);
}

}  // namespace
}  // namespace sfz
