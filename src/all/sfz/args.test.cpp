// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/args.hpp>

#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sfz/encoding.hpp>
#include <sfz/exception.hpp>
#include <sfz/format.hpp>
#include <sfz/string.hpp>

using sfz::args::store;
using sfz::args::store_const;
using sfz::args::increment;
using std::find;
using std::vector;
using testing::ElementsAre;
using testing::Eq;
using testing::Test;

namespace sfz {
namespace {

class ArgsTest : public Test {
  public:
    void pass(
            const args::Parser& parser,
            const char* argv0, const char* argv1 = NULL, const char* argv2 = NULL,
            const char* argv3 = NULL, const char* argv4 = NULL, const char* argv5 = NULL,
            const char* argv6 = NULL, const char* argv7 = NULL, const char* argv8 = NULL) {
        const char* const argv[] = {argv0, argv1, argv2, argv3, argv4, argv5, argv6, argv7, argv8};
        int argc = find(argv, argv + 9, static_cast<const char*>(NULL)) - argv;
        parser.parse_args(argc, argv);
    }

    void fail(
            const args::Parser& parser,
            const char* argv0, const char* argv1 = NULL, const char* argv2 = NULL,
            const char* argv3 = NULL, const char* argv4 = NULL, const char* argv5 = NULL,
            const char* argv6 = NULL, const char* argv7 = NULL, const char* argv8 = NULL) {
        const char* const argv[] = {argv0, argv1, argv2, argv3, argv4, argv5, argv6, argv7, argv8};
        int argc = find(argv, argv + 9, static_cast<const char*>(NULL)) - argv;
        EXPECT_THROW(parser.parse_args(argc, argv), Exception);
    }
};

TEST_F(ArgsTest, Empty) {
    args::Parser parser("Empty parser");
    pass(parser, "empty");
    pass(parser, "empty", "--");
    fail(parser, "empty", "non-empty");
    fail(parser, "empty", "-v");
    fail(parser, "empty", "--verbose");
}

struct ShortOptions {
    bool commit;
    bool aesc;
    bool aki;
    int units;
    String punctuation;

    int verbosity;

    String extension;
    String input;
    String output;
    String type;

    ShortOptions():
            commit(true),
            aesc(false),
            aki(false),
            units(1),
            punctuation("."),
            verbosity(0) { }

    void add_to(args::Parser& parser) {
        parser.add_argument("-n", store_const(commit, false))
            .help("dry run");

        parser.add_argument(utf8::decode("-æ"), store_const(aesc, true))
            .help(utf8::decode("æsc"));

        parser.add_argument(utf8::decode("-秋"), store_const(aki, true))
            .help("harvest time comes and the leaves change");

        parser.add_argument("-k", store_const(units, 1000))
            .help("print units in kilobytes");
        parser.add_argument("-m", store_const(units, 1000000))
            .help("print units in megabytes");
        parser.add_argument("-g", store_const(units, 1000000000))
            .help("print units in gigabytes");

        parser.add_argument("-?", store_const(punctuation, "?"))
            .help("hunchback - bent over and curved");
        parser.add_argument("-!", store_const(punctuation, "!"))
            .help("soldier - standing straight and at attention");

        parser.add_argument("-v", increment(verbosity))
            .help("increase verbosity level");

        parser.add_argument("-x", store(extension))
            .help("use extension");

        parser.add_argument("-i", store(input))
            .help("input file");

        parser.add_argument("-o", store(output))
            .help("output file");

        parser.add_argument("-t", store(type))
            .help("type of output");
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ShortOptions);
};

TEST_F(ArgsTest, ShortOptionsNone) {
    args::Parser parser("Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    pass(parser, "short");

    EXPECT_THAT(opts.commit, Eq(true));
    EXPECT_THAT(opts.aesc, Eq(false));
    EXPECT_THAT(opts.aki, Eq(false));
    EXPECT_THAT(opts.units, Eq(1));
    EXPECT_THAT(opts.punctuation, Eq<StringSlice>("."));
    EXPECT_THAT(opts.verbosity, Eq(0));
    EXPECT_THAT(opts.extension, Eq<StringSlice>(""));
    EXPECT_THAT(opts.input, Eq<StringSlice>(""));
    EXPECT_THAT(opts.output, Eq<StringSlice>(""));
    EXPECT_THAT(opts.type, Eq<StringSlice>(""));
}

TEST_F(ArgsTest, ShortOptionsSeparate) {
    args::Parser parser("Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    pass(parser, "short", "-n", "-v", "-o", "out", "-i", "in", "-v");

    EXPECT_THAT(opts.commit, Eq(false));
    EXPECT_THAT(opts.aesc, Eq(false));
    EXPECT_THAT(opts.aki, Eq(false));
    EXPECT_THAT(opts.units, Eq(1));
    EXPECT_THAT(opts.punctuation, Eq<StringSlice>("."));
    EXPECT_THAT(opts.verbosity, Eq(2));
    EXPECT_THAT(opts.extension, Eq<StringSlice>(""));
    EXPECT_THAT(opts.input, Eq<StringSlice>("in"));
    EXPECT_THAT(opts.output, Eq<StringSlice>("out"));
    EXPECT_THAT(opts.type, Eq<StringSlice>(""));
}

TEST_F(ArgsTest, ShortOptionsAll) {
    args::Parser parser("Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    pass(parser, "short", "-næ秋", "-kmg", "-?!", "-vvvv", "-xtxt", "-iin", "-oout", "-tTEXT");

    EXPECT_THAT(opts.commit, Eq(false));
    EXPECT_THAT(opts.aesc, Eq(true));
    EXPECT_THAT(opts.aki, Eq(true));
    EXPECT_THAT(opts.units, Eq(1000000000));
    EXPECT_THAT(opts.punctuation, Eq<StringSlice>("!"));
    EXPECT_THAT(opts.verbosity, Eq(4));
    EXPECT_THAT(opts.extension, Eq<StringSlice>("txt"));
    EXPECT_THAT(opts.input, Eq<StringSlice>("in"));
    EXPECT_THAT(opts.output, Eq<StringSlice>("out"));
    EXPECT_THAT(opts.type, Eq<StringSlice>("TEXT"));
}

TEST_F(ArgsTest, ShortOptionsFail) {
    args::Parser parser("Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    fail(parser, "short", "extra");
    fail(parser, "short", "-a");
    fail(parser, "short", "-t");
    fail(parser, "short", "--t");
    fail(parser, "short", "--t=TEXT");

    fail(parser, "short", "-kmg", "-iin", "-oout", "extra");
    fail(parser, "short", "-kmg", "-iin", "-oout", "-a");
    fail(parser, "short", "-kmg", "-iin", "-oout", "--t");
}

struct Greeter {
    Greeter():
            _exclamation_point(true),
            _greeting("Hello"),
            _name("world"),
            _times(1) { }

    void add_to(args::Parser& parser) {
        parser.add_argument("--normal", store_const(_exclamation_point, false))
            .help("greet with a period");
        parser.add_argument("--exclamation-point", store_const(_exclamation_point, true))
            .help("greet with an exclamation point");

        parser.add_argument("--hello", store_const(_greeting, "Hello"))
            .help("greet in English");
        parser.add_argument(
                utf8::decode("--ελληνικά"), store_const(_greeting, utf8::decode("Καλημέρα")))
            .help("greet in Greek");
        parser.add_argument(
                utf8::decode("--日本語"), store_const(_greeting, utf8::decode("こんにちは")))
            .help("greet in Japanese");

        parser.add_argument("--name", store(_name))
            .help("name of person to greet");

        parser.add_argument("--again", increment(_times))
            .help("greet an additional time");
    }

    StringSlice make_greeting() {
        _result.clear();
        StringSlice suffix = _exclamation_point ? "!" : ".";
        SFZ_FOREACH(int i, range(_times), {
            _result.append(format("{0}, {1}{2}\n", _greeting, _name, suffix));
        });
        return _result;
    }

  private:
    bool _exclamation_point;
    String _greeting;
    String _name;
    int _times;

    String _result;

    DISALLOW_COPY_AND_ASSIGN(Greeter);
};

TEST_F(ArgsTest, LongOptionsNone) {
    args::Parser parser("Greeter");
    Greeter opts;
    opts.add_to(parser);
    pass(parser, "greet");
    EXPECT_THAT(opts.make_greeting(), Eq<StringSlice>("Hello, world!\n"));
}

TEST_F(ArgsTest, LongOptionsAll) {
    args::Parser parser("Greeter");
    Greeter opts;
    opts.add_to(parser);
    pass(parser, "greet", "--again", "--ελληνικά", "--name=Ελένη", "--normal", "--again");
    String expected(utf8::decode("Καλημέρα, Ελένη.\nΚαλημέρα, Ελένη.\nΚαλημέρα, Ελένη.\n"));
    EXPECT_THAT(opts.make_greeting(), Eq<StringSlice>(expected));
}

struct ArgumentsOnly {
    String one;
    String two;
    String three;

    ArgumentsOnly() { }

    void add_to(args::Parser& parser) {
        parser.add_argument("one", store(one))
            .help("first argument");
        parser.add_argument("two", store(two))
            .help("second argument");
        parser.add_argument("three", store(three))
            .help("third argument");
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ArgumentsOnly);
};

TEST_F(ArgsTest, ArgumentsEmpty) {
    args::Parser parser("Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "args");
    EXPECT_THAT(opts.one, Eq<StringSlice>(""));
    EXPECT_THAT(opts.two, Eq<StringSlice>(""));
    EXPECT_THAT(opts.three, Eq<StringSlice>(""));
}

TEST_F(ArgsTest, ArgumentsSome) {
    args::Parser parser("Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "args", "1", "2");
    EXPECT_THAT(opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(opts.two, Eq<StringSlice>("2"));
    EXPECT_THAT(opts.three, Eq<StringSlice>(""));
}

TEST_F(ArgsTest, ArgumentsAll) {
    args::Parser parser("Arguments all");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "args", "1", "2", "3");
    EXPECT_THAT(opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(opts.two, Eq<StringSlice>("2"));
    EXPECT_THAT(opts.three, Eq<StringSlice>("3"));
}

TEST_F(ArgsTest, ArgumentsDash) {
    args::Parser parser("Arguments all");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "args", "1", "-", "3");
    EXPECT_THAT(opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(opts.two, Eq<StringSlice>("-"));
    EXPECT_THAT(opts.three, Eq<StringSlice>("3"));
}

TEST_F(ArgsTest, ArgumentsDashDash) {
    args::Parser parser("Arguments all");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "args", "1", "--", "-2", "--3");
    EXPECT_THAT(opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(opts.two, Eq<StringSlice>("-2"));
    EXPECT_THAT(opts.three, Eq<StringSlice>("--3"));
}

TEST_F(ArgsTest, ArgumentsFail) {
    args::Parser parser("Arguments all");
    ArgumentsOnly opts;
    opts.add_to(parser);
    fail(parser, "args", "-s");
    fail(parser, "args", "--long");
    fail(parser, "args", "1", "2", "3", "4");
}

class CutTool {
  public:
    CutTool():
            _delimiter("\t") { }

    void add_to(args::Parser& parser) {
        parser.add_argument("string", store(_string))
            .help("string to split");
        parser.add_argument("-d", "--delimiter", store(_delimiter))
            .help("use this as the field delimiter instead of tab");
    }

    vector<StringSlice>& cut() {
        StringSlice remainder = _string;
        bool next = true;
        while (next) {
            StringSlice token;
            next = partition(token, _delimiter, remainder);
            _result.push_back(token);
        }
        return _result;
    }

  private:
    String _string;
    String _delimiter;

    vector<StringSlice> _result;

    DISALLOW_COPY_AND_ASSIGN(CutTool);
};

TEST_F(ArgsTest, CutSimple) {
    args::Parser parser("Arguments all");
    CutTool opts;
    opts.add_to(parser);
    pass(parser, "args", "1\t2\t3");
    ASSERT_THAT(opts.cut(), ElementsAre("1", "2", "3"));
}

TEST_F(ArgsTest, CutShort) {
    args::Parser parser("Arguments all");
    CutTool opts;
    opts.add_to(parser);
    pass(parser, "args", "doo-wop", "-d-");
    ASSERT_THAT(opts.cut(), ElementsAre("doo", "wop"));
}

TEST_F(ArgsTest, CutLong) {
    args::Parser parser("Arguments all");
    CutTool opts;
    opts.add_to(parser);
    pass(parser, "args", "--delimiter=an", "A man, a plan, a canal, Panama");
    ASSERT_THAT(opts.cut(), ElementsAre("A m", ", a pl", ", a c", "al, P", "ama"));
}

}  // namespace
}  // namespace sfz
