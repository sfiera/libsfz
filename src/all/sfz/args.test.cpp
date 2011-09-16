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
#include <sfz/optional.hpp>
#include <sfz/string.hpp>

using sfz::args::append;
using sfz::args::store;
using sfz::args::store_const;
using sfz::args::increment;
using std::find;
using std::vector;
using testing::ElementsAre;
using testing::Eq;
using testing::Test;

namespace sfz {

void PrintTo(const String& s, std::ostream* ostr) {
    String quoted(quote(s));
    CString c_str(quoted);
    *ostr << c_str.data();
}

void PrintTo(StringSlice s, std::ostream* ostr) {
    String quoted(quote(s));
    CString c_str(quoted);
    *ostr << c_str.data();
}

namespace {

class ArgsTest : public Test {
  public:
    void pass(
            const args::Parser& parser,
            const char* argv0 = NULL, const char* argv1 = NULL, const char* argv2 = NULL,
            const char* argv3 = NULL, const char* argv4 = NULL, const char* argv5 = NULL,
            const char* argv6 = NULL, const char* argv7 = NULL, const char* argv8 = NULL) {
        const char* const argv[] = {argv0, argv1, argv2, argv3, argv4, argv5, argv6, argv7, argv8};
        int argc = find(argv, argv + 9, static_cast<const char*>(NULL)) - argv;
        String error;
        EXPECT_THAT(parser.parse_args(argc, argv, error), Eq(true))
            << argv0 << " " << argv1 << " " << argv2 << " "
            << argv3 << " " << argv4 << " " << argv5 << " "
            << argv6 << " " << argv7 << " " << argv8;
    }

    void fail(
            const char* message,
            const args::Parser& parser,
            const char* argv0 = NULL, const char* argv1 = NULL, const char* argv2 = NULL,
            const char* argv3 = NULL, const char* argv4 = NULL, const char* argv5 = NULL,
            const char* argv6 = NULL, const char* argv7 = NULL, const char* argv8 = NULL) {
        const char* const argv[] = {argv0, argv1, argv2, argv3, argv4, argv5, argv6, argv7, argv8};
        int argc = find(argv, argv + 9, static_cast<const char*>(NULL)) - argv;
        String error;
        ASSERT_THAT(parser.parse_args(argc, argv, error), Eq(false))
            << argv0 << " " << argv1 << " " << argv2 << " "
            << argv3 << " " << argv4 << " " << argv5 << " "
            << argv6 << " " << argv7 << " " << argv8;
        EXPECT_THAT(error, Eq<StringSlice>(message))
            << argv0 << " " << argv1 << " " << argv2 << " "
            << argv3 << " " << argv4 << " " << argv5 << " "
            << argv6 << " " << argv7 << " " << argv8;
    }
};

MATCHER_P(PrintsAs, output, "") {
    String actual(arg);
    CString actual_c_str(actual);
    String expected(output);
    CString expected_c_str(expected);
    if (actual == expected) {
        *result_listener << actual_c_str.data() << " == " << expected_c_str.data();
        return true;
    } else {
        *result_listener << actual_c_str.data() << " != " << expected_c_str.data();
        return false;
    }
}

TEST_F(ArgsTest, Empty) {
    args::Parser parser("empty", "Empty parser");
    EXPECT_THAT(parser.usage(), PrintsAs("empty"));
    pass(parser);
    pass(parser, "--");
    fail("extra arguments found: \"non-empty\"", parser, "non-empty");
    fail("illegal option: -v", parser, "-v");
    fail("illegal option: --verbose", parser, "--verbose");
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
    uint16_t quality;

    ShortOptions():
            commit(true),
            aesc(false),
            aki(false),
            units(1),
            punctuation("."),
            verbosity(0),
            quality(5) { }

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
            .metavar("EXT")
            .help("use extension");

        parser.add_argument("-i", store(input))
            .metavar("FILE")
            .help("input file");

        parser.add_argument("-o", store(output))
            .metavar("FILE")
            .help("output file");

        parser.add_argument("-t", store(type))
            .metavar("TYPE")
            .help("type of output");

        parser.add_argument("-q", store(quality))
            .metavar("QUALITY")
            .help("quality");
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ShortOptions);
};

TEST_F(ArgsTest, ShortOptionsHelp) {
    args::Parser parser("short", "Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    EXPECT_THAT(parser.usage(), PrintsAs(utf8::decode(
                    "short [-!?gkmnvæ秋] [-i FILE] [-o FILE] [-q QUALITY] [-t TYPE] [-x EXT]")));
}

TEST_F(ArgsTest, ShortOptionsNone) {
    args::Parser parser("short", "Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    pass(parser);

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
    EXPECT_THAT(opts.quality, Eq(5));
}

TEST_F(ArgsTest, ShortOptionsSeparate) {
    args::Parser parser("short", "Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    pass(parser, "-n", "-v", "-o", "out", "-i", "in", "-v", "-q" "7");

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
    EXPECT_THAT(opts.quality, Eq(7));
}

TEST_F(ArgsTest, ShortOptionsAll) {
    args::Parser parser("short", "Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    pass(parser, "-næ秋", "-kmg", "-?!", "-vvvv", "-xtxt", "-iin", "-oout", "-tTEXT", "-q9");

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
    EXPECT_THAT(opts.quality, Eq(9));
}

TEST_F(ArgsTest, ShortOptionsFail) {
    args::Parser parser("short", "Short options only");
    ShortOptions opts;
    opts.add_to(parser);
    fail("extra arguments found: \"extra\"", parser, "extra");
    fail("illegal option: -a", parser, "-a");
    fail("option -t: argument required", parser, "-t");
    fail("illegal option: --t", parser, "--t");
    fail("illegal option: --t", parser, "--t=TEXT");

    fail("extra arguments found: \"extra\"", parser, "-kmg", "-iin", "-oout", "extra");
    fail("illegal option: -a", parser, "-kmg", "-iin", "-oout", "-a");
    fail("illegal option: --t", parser, "-kmg", "-iin", "-oout", "--t");

    opts.verbosity = std::numeric_limits<int>::max() - 1;
    pass(parser, "-v");
    fail("option -v: integer overflow", parser, "-vv");
    fail("option -v: integer overflow", parser, "-vvv");

    fail("option -q: invalid integer: \"x\"", parser, "-qx");
    fail("option -q: invalid integer: \"x\"", parser, "-q", "x");
    fail("option -q: invalid integer: \"-1\"", parser, "-q", "-1");
    fail("option -q: integer overflow: \"65536\"", parser, "-q", "65536");
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

TEST_F(ArgsTest, LongOptionsHelp) {
    args::Parser parser("greet", "Greeter");
    Greeter opts;
    opts.add_to(parser);
    EXPECT_THAT(parser.usage(), PrintsAs(utf8::decode(
                    "greet [--again] [--exclamation-point] [--hello] [--name=NAME] [--normal] "
                    "[--ελληνικά] [--日本語]")));
}


TEST_F(ArgsTest, LongOptionsNone) {
    args::Parser parser("greet", "Greeter");
    Greeter opts;
    opts.add_to(parser);
    pass(parser);
    EXPECT_THAT(opts.make_greeting(), Eq<StringSlice>("Hello, world!\n"));
}

TEST_F(ArgsTest, LongOptionsAll) {
    args::Parser parser("greet", "Greeter");
    Greeter opts;
    opts.add_to(parser);
    pass(parser, "--again", "--ελληνικά", "--name=Ελένη", "--normal", "--again");
    String expected(utf8::decode("Καλημέρα, Ελένη.\nΚαλημέρα, Ελένη.\nΚαλημέρα, Ελένη.\n"));
    EXPECT_THAT(opts.make_greeting(), Eq<StringSlice>(expected));
}

struct ArgumentsOnly {
    Optional<String> one;
    Optional<String> two;
    vector<int32_t> three;

    ArgumentsOnly() { }

    void add_to(args::Parser& parser) {
        parser.add_argument("one", store(one))
            .help("first argument")
            .required();
        parser.add_argument("two", store(two))
            .help("second argument");
        parser.add_argument("three", append(three))
            .help("third argument")
            .max_args(3);
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ArgumentsOnly);
};

TEST_F(ArgsTest, ArgumentsHelp) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    EXPECT_THAT(parser.usage(), PrintsAs(utf8::decode(
                    "args one [two [three [three [three]]]]")));
}

TEST_F(ArgsTest, ArgumentsEmpty) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    fail("too few arguments", parser);
}

TEST_F(ArgsTest, ArgumentsOne) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "1");
    EXPECT_THAT(*opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(opts.two.has(), Eq(false));
    EXPECT_THAT(opts.three, ElementsAre());
}

TEST_F(ArgsTest, ArgumentsTwo) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "1", "2");
    EXPECT_THAT(*opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(*opts.two, Eq<StringSlice>("2"));
    EXPECT_THAT(opts.three, ElementsAre());
}

TEST_F(ArgsTest, ArgumentsThree) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "1", "2", "3");
    EXPECT_THAT(*opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(*opts.two, Eq<StringSlice>("2"));
    EXPECT_THAT(opts.three, ElementsAre(3));
}

TEST_F(ArgsTest, ArgumentsAll) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "1", "2", "3", "4", "5");
    EXPECT_THAT(*opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(*opts.two, Eq<StringSlice>("2"));
    EXPECT_THAT(opts.three, ElementsAre(3, 4, 5));
}

TEST_F(ArgsTest, ArgumentsDash) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "1", "-", "3");
    EXPECT_THAT(*opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(*opts.two, Eq<StringSlice>("-"));
    EXPECT_THAT(opts.three, ElementsAre(3));
}

TEST_F(ArgsTest, ArgumentsDashDash) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    pass(parser, "1", "--", "--2", "-3");
    EXPECT_THAT(*opts.one, Eq<StringSlice>("1"));
    EXPECT_THAT(*opts.two, Eq<StringSlice>("--2"));
    EXPECT_THAT(opts.three, ElementsAre(-3));
}

TEST_F(ArgsTest, ArgumentsFail) {
    args::Parser parser("args", "Arguments only");
    ArgumentsOnly opts;
    opts.add_to(parser);
    fail("illegal option: -s", parser, "-s");
    fail("illegal option: --long", parser, "--long");
    fail("argument three: invalid integer: \"x\"", parser, "1", "2", "x");
    fail("argument three: invalid integer: \"x\"", parser, "1", "2", "3", "4", "x");
    fail("argument three: integer overflow: \"2147483648\"",
            parser, "1", "2", "3", "4", "2147483648");
    fail("extra arguments found: \"6\"", parser, "1", "2", "3", "4", "5", "6");
    fail("extra arguments found: \"6\" \"7\" \"8\"",
            parser, "1", "2", "3", "4", "5", "6", "7", "8");
}

class CutTool {
  public:
    CutTool():
            _limit(std::numeric_limits<int64_t>::max()),
            _delimiter("\t") { }

    void add_to(args::Parser& parser) {
        parser.add_argument("string", store(_string))
            .required()
            .help("string to split");
        parser.add_argument("-l", "--limit", store(_limit))
            .help("split at most this many times");
        parser.add_argument("-d", "--delimiter", store(_delimiter))
            .metavar("DELIM")
            .help("use this as the field delimiter instead of tab");
    }

    vector<StringSlice>& cut() {
        StringSlice remainder = _string;
        bool next = true;
        int splits = 1;
        while (next) {
            StringSlice token;
            next = partition(token, _delimiter, remainder);
            _result.push_back(token);
            ++splits;
            if (splits >= _limit) {
                _result.push_back(remainder);
                break;
            }
        }
        return _result;
    }

  private:
    String _string;
    int64_t _limit;
    String _delimiter;

    vector<StringSlice> _result;

    DISALLOW_COPY_AND_ASSIGN(CutTool);
};

TEST_F(ArgsTest, CutHelp) {
    args::Parser parser("cut", "A tool like cut(1)");
    CutTool opts;
    opts.add_to(parser);
    EXPECT_THAT(parser.usage(), PrintsAs(utf8::decode(
                    "cut [-d DELIM] [-l LIMIT] [--delimiter=DELIM] [--limit=LIMIT] string")));
}

TEST_F(ArgsTest, CutSimple) {
    args::Parser parser("cut", "A tool like cut(1)");
    CutTool opts;
    opts.add_to(parser);
    pass(parser, "1\t2\t3");
    ASSERT_THAT(opts.cut(), ElementsAre("1", "2", "3"));
}

TEST_F(ArgsTest, CutShort) {
    args::Parser parser("cut", "A tool like cut(1)");
    CutTool opts;
    opts.add_to(parser);
    pass(parser, "doo-wop", "-d-");
    ASSERT_THAT(opts.cut(), ElementsAre("doo", "wop"));
}

TEST_F(ArgsTest, CutLong) {
    args::Parser parser("cut", "A tool like cut(1)");
    CutTool opts;
    opts.add_to(parser);
    pass(parser, "--delimiter=an", "A man, a plan, a canal, Panama", "-l4");
    ASSERT_THAT(opts.cut(), ElementsAre("A m", ", a pl", ", a c", "al, Panama"));
}

}  // namespace
}  // namespace sfz
