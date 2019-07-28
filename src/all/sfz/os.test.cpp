// Copyright (c) 2010-2019 The libsfz Authors
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/os.hpp>

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pn/output>

using testing::_;
using testing::Eq;
using testing::Expectation;
using testing::InSequence;
using testing::Ne;
using testing::StrictMock;
using testing::Test;

namespace pn {
std::ostream& operator<<(std::ostream& ostr, const pn::string& s) {
    pn::string q = pn::dump(s, pn::dump_short);
    return ostr << std::string(q.data(), q.size());
}
}  // namespace pn

namespace pn {
std::ostream& operator<<(std::ostream& ostr, pn::string_view s) {
    pn::string q = pn::dump(s, pn::dump_short);
    return ostr << std::string(q.data(), q.size());
}
}  // namespace pn

namespace sfz {
namespace {

typedef Test OsTest;

std::pair<pn::string_view, pn::string_view> strings(
        pn::string_view first, pn::string_view second) {
    return {first, second};
}

// Test path::splitdrive() with many different inputs.
TEST_F(OsTest, SplitDrive) {
    const struct {
        pn::string_view path;
        pn::string_view drive, local;
    } cases[] = {
            {"", "", ""},
            {".", "", "."},
            {"/", "", "/"},
            {"\\", "", "\\"},
            {"C", "", "C"},

            {"path-to-file", "", "path-to-file"},
            {"path/to/file", "", "path/to/file"},
            {"path\\to\\file", "", "path\\to\\file"},

            {"C:", "C:", ""},
            {"C:\\", "C:", "\\"},
            {"C:/", "C:", "/"},
            {"C:path", "C:", "path"},
            {"C:\\path", "C:", "\\path"},
            {"C:/path", "C:", "/path"},

            {"\\", "", "\\"},
            {"\\\\", "", "\\\\"},
            {"\\\\\\", "", "\\\\\\"},
            {"\\\\\\\\", "", "\\\\\\\\"},

            {"\\\\machine", "", "\\\\machine"},
            {"\\\\machine\\", "", "\\\\machine\\"},
            {"\\\\\\mount", "", "\\\\\\mount"},
            {"\\\\\\mount\\", "", "\\\\\\mount\\"},

            {"\\\\machine\\mount", "\\\\machine\\mount", ""},
            {"\\\\machine\\mount\\", "\\\\machine\\mount", "\\"},
            {"\\\\machine\\mount\\file", "\\\\machine\\mount", "\\file"},
            {"\\\\machine\\mount\\dir\\", "\\\\machine\\mount", "\\dir\\"},
            {"\\\\machine\\mount\\dir\\file", "\\\\machine\\mount", "\\dir\\file"},
    };
    for (auto c : cases) {
        auto drive_local = path::splitdrive(c.path);
        auto drive       = drive_local.first;
        auto local       = drive_local.second;
#ifdef _WIN32
        EXPECT_THAT(drive, Eq(c.drive));
        EXPECT_THAT(local, Eq(c.local));
#else
        EXPECT_THAT(drive, Eq(""));
        EXPECT_THAT(local, Eq(c.path));
#endif
    }
}

// Test path::{split,dirname,basename}() with many different inputs.
TEST_F(OsTest, Split) {
    enum class on {
        this_platform,
        not_this_platform,
        neither = not_this_platform,
        both    = this_platform,
#ifdef _WIN32
        windows = this_platform,
        posix   = not_this_platform,
#else
        posix   = this_platform,
        windows = not_this_platform,
#endif
    };
    const struct {
        pn::string_view path;
        pn::string_view dir, base;
        on              canonical;
    } cases[] = {
            {"/", "/", "/", on::both},
            {"/aesc", "/", "aesc", on::both},
            {"/aesc/", "/", "aesc", on::neither},
            {"/aesc/wynn", "/aesc", "wynn", on::posix},
            {"/aesc/wynn/", "/aesc", "wynn", on::neither},
            {"/aesc/wynn/.", "/aesc/wynn", ".", on::posix},
            {"/aesc/wynn/..", "/aesc/wynn", "..", on::posix},

            {"", ".", "", on::neither},
            {"aesc", ".", "aesc", on::neither},
            {"aesc/", ".", "aesc", on::neither},
            {"aesc/wynn", "aesc", "wynn", on::posix},
            {"aesc/wynn/", "aesc", "wynn", on::neither},
            {"aesc/wynn/.", "aesc/wynn", ".", on::posix},
            {"aesc/wynn/..", "aesc/wynn", "..", on::posix},

#ifdef _WIN32
            {"\\", "\\", "\\", on::windows},
            {"\\aesc", "\\", "aesc", on::windows},
            {"\\aesc\\", "\\", "aesc", on::neither},
            {"\\aesc\\wynn", "\\aesc", "wynn", on::windows},
            {"\\aesc\\wynn\\", "\\aesc", "wynn", on::neither},
            {"\\aesc\\wynn\\.", "\\aesc\\wynn", ".", on::windows},
            {"\\aesc\\wynn\\..", "\\aesc\\wynn", "..", on::windows},

            {"", ".", "", on::neither},
            {"aesc", ".", "aesc", on::neither},
            {"aesc\\", ".", "aesc", on::neither},
            {"aesc\\wynn", "aesc", "wynn", on::windows},
            {"aesc\\wynn\\", "aesc", "wynn", on::neither},
            {"aesc\\wynn\\.", "aesc\\wynn", ".", on::windows},
            {"aesc\\wynn\\..", "aesc\\wynn", "..", on::windows},

            {"C:", "C:", "", on::windows},
            {"C:aesc", "C:", "aesc", on::windows},
            {"C:aesc\\", "C:", "aesc", on::neither},
            {"C:aesc\\wynn", "C:aesc", "wynn", on::windows},
            {"C:aesc\\wynn\\", "C:aesc", "wynn", on::neither},
            {"C:aesc\\wynn\\.", "C:aesc\\wynn", ".", on::windows},
            {"C:aesc\\wynn\\..", "C:aesc\\wynn", "..", on::windows},

            {"C:\\", "C:\\", "\\", on::windows},
            {"C:\\aesc", "C:\\", "aesc", on::windows},
            {"C:\\aesc\\", "C:\\", "aesc", on::neither},
            {"C:\\aesc\\wynn", "C:\\aesc", "wynn", on::windows},
            {"C:\\aesc\\wynn\\", "C:\\aesc", "wynn", on::neither},
            {"C:\\aesc\\wynn\\.", "C:\\aesc\\wynn", ".", on::windows},
            {"C:\\aesc\\wynn\\..", "C:\\aesc\\wynn", "..", on::windows},

            {"\\\\", "\\", "\\", on::neither},
            {"\\\\\\", "\\", "\\", on::neither},
            {"\\\\\\\\", "\\", "\\", on::neither},
            {"\\\\aesc", "\\\\", "aesc", on::windows},
            {"\\\\aesc\\", "\\\\", "aesc", on::neither},
            {"\\\\aesc\\wynn", "\\\\aesc\\wynn", "", on::windows},
            {"\\\\aesc\\wynn\\", "\\\\aesc\\wynn\\", "\\", on::windows},
            {"\\\\aesc\\wynn\\eth", "\\\\aesc\\wynn\\", "eth", on::windows},
            {"\\\\aesc\\wynn\\eth\\", "\\\\aesc\\wynn\\", "eth", on::neither},
#endif
    };
    for (auto c : cases) {
        EXPECT_THAT(path::dirname(c.path), Eq(c.dir)) << c.path;
        EXPECT_THAT(path::basename(c.path), Eq(c.base)) << c.path;
        EXPECT_THAT(path::split(c.path), Eq(std::make_pair(c.dir, c.base))) << c.path;

        pn::string joined = path::join(path::dirname(c.path), path::basename(c.path));
        if (c.canonical == on::this_platform) {
            EXPECT_THAT(joined, Eq(c.path));
        } else {
            EXPECT_THAT(joined, Ne(c.path));
        }
    }
}

TEST_F(OsTest, Join) {
#ifdef _WIN32
    EXPECT_THAT(path::join("a"), Eq("a"));
    EXPECT_THAT(path::join("a", "b"), Eq("a\\b"));
    EXPECT_THAT(path::join("a", "b", "c"), Eq("a\\b\\c"));
    EXPECT_THAT(path::join("a", "b", "c", "d"), Eq("a\\b\\c\\d"));

    EXPECT_THAT(path::join("a", "/b", "c"), Eq("/b\\c"));
    EXPECT_THAT(path::join("a", "\\b", "c"), Eq("\\b\\c"));
    EXPECT_THAT(path::join("a", "b", "/c"), Eq("/c"));
    EXPECT_THAT(path::join("a", "b", "\\c"), Eq("\\c"));
    EXPECT_THAT(path::join("a", "/b", "/c"), Eq("/c"));
    EXPECT_THAT(path::join("a", "\\b", "/c"), Eq("/c"));
    EXPECT_THAT(path::join("a", "/b", "\\c"), Eq("\\c"));

    EXPECT_THAT(path::join("a", "B:", "/c"), Eq("B:/c"));
    EXPECT_THAT(path::join("a", "/b", "C:"), Eq("C:"));
#else
    EXPECT_THAT(path::join("a"), Eq("a"));
    EXPECT_THAT(path::join("a", "b"), Eq("a/b"));
    EXPECT_THAT(path::join("a", "b", "c"), Eq("a/b/c"));
    EXPECT_THAT(path::join("a", "b", "c", "d"), Eq("a/b/c/d"));

    EXPECT_THAT(path::join("a", "/b", "c"), Eq("/b/c"));
    EXPECT_THAT(path::join("a", "/b", "c"), Eq("/b/c"));
    EXPECT_THAT(path::join("a", "b", "/c"), Eq("/c"));
    EXPECT_THAT(path::join("a", "b", "/c"), Eq("/c"));
    EXPECT_THAT(path::join("a", "/b", "/c"), Eq("/c"));
    EXPECT_THAT(path::join("a", "/b", "/c"), Eq("/c"));
    EXPECT_THAT(path::join("a", "/b", "/c"), Eq("/c"));

    EXPECT_THAT(path::join("a", "B:", "/c"), Eq("/c"));
    EXPECT_THAT(path::join("a", "/b", "C:"), Eq("/b/C:"));
#endif
}

class MockTreeWalker : public TreeWalker {
  public:
    MOCK_CONST_METHOD2(pre_directory, void(pn::string_view name, const Stat& st));
    MOCK_CONST_METHOD2(cycle_directory, void(pn::string_view name, const Stat& st));
    MOCK_CONST_METHOD2(post_directory, void(pn::string_view name, const Stat& st));
    MOCK_CONST_METHOD2(file, void(pn::string_view name, const Stat& st));
    MOCK_CONST_METHOD2(symlink, void(pn::string_view name, const Stat& st));
    MOCK_CONST_METHOD2(broken_symlink, void(pn::string_view name, const Stat& st));
    MOCK_CONST_METHOD2(other, void(pn::string_view name, const Stat& st));
};

MATCHER(IsDirStat, "") { return (arg.st_mode & S_IFMT) == S_IFDIR; }
MATCHER(IsFileStat, "") { return (arg.st_mode & S_IFMT) == S_IFREG; }
MATCHER(IsLinkStat, "") {
#ifdef _WIN32
    return false;
#else
    return (arg.st_mode & S_IFMT) == S_IFLNK;
#endif
}
MATCHER(IsFifoStat, "") { return (arg.st_mode & S_IFMT) == S_IFIFO; }

namespace {

class scoped_chdir {
  public:
    scoped_chdir(pn::string_view to) : _origin{getcwd()} { chdir(to); }
    ~scoped_chdir() { chdir(_origin); }

  private:
    pn::string _origin;
};

}  // namespace

// Create a temporary directory with TemporaryDirectory, then fill it with several directories,
// files, and symlinks.  Try walking the directory with a few different options; then, delete some
// of the files and directories and try walking again.
//
// This exercises most of os.hpp, aside from the simple functions tested above, and some of the
// stranger cases during walk().
TEST_F(OsTest, Hierarchy) {
    TemporaryDirectory dir("os-test");

    ASSERT_THAT(path::exists(dir.path()), Eq(true));
    ASSERT_THAT(path::isdir(dir.path()), Eq(true));
    ASSERT_THAT(path::isfile(dir.path()), Eq(false));

    scoped_chdir d{dir.path()};
    makedirs("./roman/upper", 0700);
    close(open("./roman/upper/A", O_WRONLY | O_CREAT | O_EXCL, 0600));
    close(open("./roman/upper/B", O_WRONLY | O_CREAT | O_EXCL, 0600));
    close(open("./roman/upper/Z", O_WRONLY | O_CREAT | O_EXCL, 0600));
    makedirs("./roman/lower", 0700);
    close(open("./roman/README", O_WRONLY | O_CREAT | O_EXCL, 0600));
    makedirs("./cyrillic/upper", 0700);
    makedirs("./cyrillic/lower", 0700);
    close(open("./cyrillic/README", O_WRONLY | O_CREAT | O_EXCL, 0600));

#ifndef _WIN32
    symlink("../../roman/upper/A", "./cyrillic/upper/A");
    symlink("../../roman/lower/a", "./cyrillic/lower/a");
#endif  // _WIN32

    EXPECT_THAT(path::exists("./roman/upper"), Eq(true));
    EXPECT_THAT(path::isdir("./roman/upper"), Eq(true));
    EXPECT_THAT(path::isfile("./roman/upper"), Eq(false));
    EXPECT_THAT(path::islink("./roman/upper"), Eq(false));

    EXPECT_THAT(path::exists("./roman/upper/A"), Eq(true));
    EXPECT_THAT(path::isdir("./roman/upper/A"), Eq(false));
    EXPECT_THAT(path::isfile("./roman/upper/A"), Eq(true));
    EXPECT_THAT(path::islink("./roman/upper/A"), Eq(false));

#ifndef _WIN32
    EXPECT_THAT(path::exists("./cyrillic/upper/A"), Eq(true));
    EXPECT_THAT(path::isdir("./cyrillic/upper/A"), Eq(false));
    EXPECT_THAT(path::isfile("./cyrillic/upper/A"), Eq(true));
    EXPECT_THAT(path::islink("./cyrillic/upper/A"), Eq(true));

    EXPECT_THAT(path::exists("./cyrillic/lower/a"), Eq(false));
    EXPECT_THAT(path::isdir("./cyrillic/lower/a"), Eq(false));
    EXPECT_THAT(path::isfile("./cyrillic/lower/a"), Eq(false));
    EXPECT_THAT(path::islink("./cyrillic/lower/a"), Eq(true));
#endif  // _WIN32

    EXPECT_THAT(path::exists("./cyrillic/lower/Z"), Eq(false));
    EXPECT_THAT(path::isdir("./cyrillic/lower/Z"), Eq(false));
    EXPECT_THAT(path::isfile("./cyrillic/lower/Z"), Eq(false));
    EXPECT_THAT(path::islink("./cyrillic/lower/Z"), Eq(false));

    {
        testing::StrictMock<testing::MockFunction<void(pn::string_view)>> f;
        EXPECT_CALL(f, Call(pn::string_view("roman")));
        EXPECT_CALL(f, Call(pn::string_view("cyrillic")));

        for (const auto& ent : scandir(".")) {
            f.Call(ent.name);
        }
    }

    {
        testing::StrictMock<testing::MockFunction<void(pn::string_view)>> f;
        EXPECT_CALL(f, Call(pn::string_view("A")));
        EXPECT_CALL(f, Call(pn::string_view("B")));
        EXPECT_CALL(f, Call(pn::string_view("Z")));

        for (const auto& ent : scandir("roman/upper")) {
            f.Call(ent.name);
        }
    }

    EXPECT_THROW(scandir("arabic"), std::runtime_error);

#ifdef _WIN32
    {
        StrictMock<MockTreeWalker> walker;
        Expectation                pre_roman =
                EXPECT_CALL(walker, pre_directory(pn::string_view("roman"), IsDirStat()));

        Expectation pre_upper =
                EXPECT_CALL(walker, pre_directory(pn::string_view("roman\\upper"), IsDirStat()))
                        .After(pre_roman);
        Expectation upper_a =
                EXPECT_CALL(walker, file(pn::string_view("roman\\upper\\A"), IsFileStat()))
                        .After(pre_upper);
        Expectation upper_b =
                EXPECT_CALL(walker, file(pn::string_view("roman\\upper\\B"), IsFileStat()))
                        .After(pre_upper);
        Expectation upper_z =
                EXPECT_CALL(walker, file(pn::string_view("roman\\upper\\Z"), IsFileStat()))
                        .After(pre_upper);
        Expectation post_upper =
                EXPECT_CALL(walker, post_directory(pn::string_view("roman\\upper"), IsDirStat()))
                        .After(upper_a, upper_b, upper_z);

        Expectation pre_lower =
                EXPECT_CALL(walker, pre_directory(pn::string_view("roman\\lower"), IsDirStat()))
                        .After(pre_roman);
        Expectation post_lower =
                EXPECT_CALL(walker, post_directory(pn::string_view("roman\\lower"), IsDirStat()))
                        .After(pre_lower);

        Expectation readme =
                EXPECT_CALL(walker, file(pn::string_view("roman\\README"), IsFileStat()))
                        .After(pre_roman);

        Expectation post_roman =
                EXPECT_CALL(walker, post_directory(pn::string_view("roman"), IsDirStat()))
                        .After(post_upper, post_lower, readme);

        walk("roman", WALK_PHYSICAL, walker);
    }

#else
    {
        StrictMock<MockTreeWalker> walker;
        InSequence s;
        EXPECT_CALL(walker, pre_directory(pn::string_view("roman"), IsDirStat()));
        EXPECT_CALL(walker, file(pn::string_view("roman/README"), IsFileStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("roman/lower"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("roman/lower"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("roman/upper"), IsDirStat()));
        EXPECT_CALL(walker, file(pn::string_view("roman/upper/A"), IsFileStat()));
        EXPECT_CALL(walker, file(pn::string_view("roman/upper/B"), IsFileStat()));
        EXPECT_CALL(walker, file(pn::string_view("roman/upper/Z"), IsFileStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("roman/upper"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("roman"), IsDirStat()));
        walk("roman", WALK_PHYSICAL, walker);
    }

    {
        MockTreeWalker walker;
        InSequence s;
        EXPECT_CALL(walker, pre_directory(pn::string_view("cyrillic"), IsDirStat()));
        EXPECT_CALL(walker, file(pn::string_view("cyrillic/README"), IsFileStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("cyrillic/lower"), IsDirStat()));
        EXPECT_CALL(walker, symlink(pn::string_view("cyrillic/lower/a"), IsLinkStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("cyrillic/lower"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, symlink(pn::string_view("cyrillic/upper/A"), IsLinkStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("cyrillic"), IsDirStat()));
        walk("cyrillic", WALK_PHYSICAL, walker);
    }

    {
        MockTreeWalker walker;
        InSequence s;
        EXPECT_CALL(walker, pre_directory(pn::string_view("cyrillic"), IsDirStat()));
        EXPECT_CALL(walker, file(pn::string_view("cyrillic/README"), IsFileStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("cyrillic/lower"), IsDirStat()));
        EXPECT_CALL(walker, broken_symlink(pn::string_view("cyrillic/lower/a"), IsLinkStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("cyrillic/lower"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, file(pn::string_view("cyrillic/upper/A"), IsFileStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("cyrillic"), IsDirStat()));
        walk("cyrillic", WALK_LOGICAL, walker);
    }

    unlink("cyrillic/lower/a");
    unlink("cyrillic/README");
    unlink("roman/README");
    rmdir("cyrillic/lower");
    rmtree("roman/upper");

    {
        MockTreeWalker walker;
        InSequence s;
        EXPECT_CALL(walker, pre_directory(pn::string_view("."), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("./cyrillic"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("./cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, broken_symlink(pn::string_view("./cyrillic/upper/A"), IsLinkStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("./cyrillic/upper"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("./cyrillic"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("./roman"), IsDirStat()));
        EXPECT_CALL(walker, pre_directory(pn::string_view("./roman/lower"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("./roman/lower"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("./roman"), IsDirStat()));
        EXPECT_CALL(walker, post_directory(pn::string_view("."), IsDirStat()));
        walk(".", WALK_LOGICAL, walker);
    }
#endif  // _WIN32
}

// Exercise the remaining bits of os.hpp: mkfifo, as well as walk() calls which include files of
// type other() and cycle_directory().
TEST_F(OsTest, WalkOther) {
    TemporaryDirectory dir("os-test");

    ASSERT_THAT(path::exists(dir.path()), Eq(true));
    ASSERT_THAT(path::isdir(dir.path()), Eq(true));
    ASSERT_THAT(path::isfile(dir.path()), Eq(false));

    scoped_chdir d{dir.path()};
    makedirs("./aesc/wynn", 0700);
#ifndef _WIN32
    symlink("../..", "./aesc/wynn/eth");
    mkfifo("./aesc/thorn", 0600);
#endif

    MockTreeWalker walker;

    InSequence s;
#ifdef _WIN32
    EXPECT_CALL(walker, pre_directory(pn::string_view("."), IsDirStat()));
    EXPECT_CALL(walker, pre_directory(pn::string_view(".\\aesc"), IsDirStat()));
    EXPECT_CALL(walker, pre_directory(pn::string_view(".\\aesc\\wynn"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(pn::string_view(".\\aesc\\wynn"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(pn::string_view(".\\aesc"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(pn::string_view("."), IsDirStat()));
#else
    EXPECT_CALL(walker, pre_directory(pn::string_view("."), IsDirStat()));
    EXPECT_CALL(walker, pre_directory(pn::string_view("./aesc"), IsDirStat()));
    EXPECT_CALL(walker, other(pn::string_view("./aesc/thorn"), IsFifoStat()));
    EXPECT_CALL(walker, pre_directory(pn::string_view("./aesc/wynn"), IsDirStat()));
    EXPECT_CALL(walker, cycle_directory(pn::string_view("./aesc/wynn/eth"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(pn::string_view("./aesc/wynn"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(pn::string_view("./aesc"), IsDirStat()));
    EXPECT_CALL(walker, post_directory(pn::string_view("."), IsDirStat()));
#endif

    walk(".", WALK_LOGICAL, walker);
}

}  // namespace
}  // namespace sfz
