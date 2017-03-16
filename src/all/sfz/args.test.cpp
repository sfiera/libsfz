// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/args.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <sfz/encoding.hpp>
#include <sfz/macros.hpp>
#include <sfz/optional.hpp>
#include <sfz/range.hpp>
#include <sfz/string.hpp>
#include <stdexcept>

using std::find;
using std::vector;
using testing::ElementsAre;
using testing::Eq;
using testing::IsEmpty;
using testing::Test;

namespace sfz {

namespace {

void unnest_exception_string(pn::string_ref s, std::exception& e) {
    if (!s.empty()) {
        s += ": ";
    }
    s += e.what();
    try {
        std::rethrow_if_nested(e);
    } catch (std::exception& nested) {
        unnest_exception_string(s, nested);
    } catch (...) {
        s += ": unknown exception";
    }
}

pn::string error_string(std::exception& e) {
    pn::string s;
    unnest_exception_string(s, e);
    return s;
}

class ArgsTest : public Test {
  public:
    template <typename... arguments>
    void pass(const args::callbacks& callbacks, const arguments&... args) {
        char* const argv[] = {strdup(args)...};
        int         argc   = sizeof...(args);
        String      error;
        try {
            args::parse(argc, argv, callbacks);
        } catch (std::exception& e) {
            FAIL() << ::testing::PrintToString(argv) << " threw: " << error_string(e).cpp_str();
        } catch (...) {
            FAIL() << ::testing::PrintToString(argv) << " threw weirdness";
        }
        for (int i = 0; i < argc; ++i) {
            free(argv[i]);
        }
    }

    template <typename... arguments>
    void fail(const char* message, const args::callbacks& callbacks, const arguments&... args) {
        char* const argv[] = {strdup(args)...};
        int         argc   = sizeof...(args);
        String      error;
        try {
            args::parse(argc, argv, callbacks);
        } catch (std::runtime_error& e) {
            EXPECT_THAT(error_string(e).cpp_str(), Eq(message)) << ::testing::PrintToString(argv);
            goto free;
        } catch (...) {
            FAIL() << ::testing::PrintToString(argv) << " threw weirdness";
            goto free;
        }
        FAIL() << ::testing::PrintToString(argv) << " didn't throw";
    free:
        for (int i = 0; i < argc; ++i) {
            free(argv[i]);
        }
    }
};

TEST_F(ArgsTest, Empty) {
    args::callbacks empty{};

    pass(empty);
    pass(empty, "--");
    fail("extra arguments found: \"non-empty\"", empty, "non-empty");
    fail("illegal option: -v", empty, "-v");
    fail("illegal option: --verbose", empty, "--verbose");
}

struct ShortOptions {
    bool     commit;
    bool     aesc;
    bool     aki;
    int      units;
    pn::rune punctuation;

    int verbosity;

    pn::string extension;
    pn::string input;
    pn::string output;
    pn::string type;
    uint16_t   quality;

    ShortOptions()
            : commit(true),
              aesc(false),
              aki(false),
              units(1),
              punctuation{'.'},
              verbosity(0),
              quality(5) {}

    args::callbacks callbacks() {
        args::callbacks callbacks;
        callbacks.short_option = [this](pn::rune opt, args::callbacks::get_value_f get_value) {
            switch (opt.value()) {
                case 'n': commit      = false; return true;
                case L'æ': aesc       = true; return true;
                case L'秋': aki       = true; return true;
                case 'k': units       = 1000; return true;
                case 'm': units       = 1000000; return true;
                case 'g': units       = 1000000000; return true;
                case '?': punctuation = pn::rune{'?'}; return true;
                case '!': punctuation = pn::rune{'!'}; return true;
                case 'v': args::increment_option(&verbosity, 1); return true;
                case 'x': extension = get_value().copy(); return true;
                case 'i': input     = get_value().copy(); return true;
                case 'o': output    = get_value().copy(); return true;
                case 't': type      = get_value().copy(); return true;
                case 'q': args::integer_option(get_value(), &quality); return true;
                default: return false;
            }
        };
        return callbacks;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ShortOptions);
};

TEST_F(ArgsTest, ShortOptionsNone) {
    ShortOptions opts;
    pass(opts.callbacks());

    EXPECT_THAT(opts.commit, Eq(true));
    EXPECT_THAT(opts.aesc, Eq(false));
    EXPECT_THAT(opts.aki, Eq(false));
    EXPECT_THAT(opts.units, Eq(1));
    EXPECT_THAT(opts.punctuation, Eq("."));
    EXPECT_THAT(opts.verbosity, Eq(0));
    EXPECT_THAT(opts.extension, Eq(""));
    EXPECT_THAT(opts.input, Eq(""));
    EXPECT_THAT(opts.output, Eq(""));
    EXPECT_THAT(opts.type, Eq(""));
    EXPECT_THAT(opts.quality, Eq(5));
}

TEST_F(ArgsTest, ShortOptionsSeparate) {
    ShortOptions opts;
    pass(opts.callbacks(), "-n", "-v", "-o", "out", "-i", "in", "-v", "-q", "7");

    EXPECT_THAT(opts.commit, Eq(false));
    EXPECT_THAT(opts.aesc, Eq(false));
    EXPECT_THAT(opts.aki, Eq(false));
    EXPECT_THAT(opts.units, Eq(1));
    EXPECT_THAT(opts.punctuation, Eq("."));
    EXPECT_THAT(opts.verbosity, Eq(2));
    EXPECT_THAT(opts.extension, Eq(""));
    EXPECT_THAT(opts.input, Eq("in"));
    EXPECT_THAT(opts.output, Eq("out"));
    EXPECT_THAT(opts.type, Eq(""));
    EXPECT_THAT(opts.quality, Eq(7));
}

TEST_F(ArgsTest, ShortOptionsAll) {
    ShortOptions opts;
    pass(opts.callbacks(), "-næ秋", "-kmg", "-?!", "-vvvv", "-xtxt", "-iin", "-oout", "-tTEXT",
         "-q9");

    EXPECT_THAT(opts.commit, Eq(false));
    EXPECT_THAT(opts.aesc, Eq(true));
    EXPECT_THAT(opts.aki, Eq(true));
    EXPECT_THAT(opts.units, Eq(1000000000));
    EXPECT_THAT(opts.punctuation, Eq("!"));
    EXPECT_THAT(opts.verbosity, Eq(4));
    EXPECT_THAT(opts.extension, Eq("txt"));
    EXPECT_THAT(opts.input, Eq("in"));
    EXPECT_THAT(opts.output, Eq("out"));
    EXPECT_THAT(opts.type, Eq("TEXT"));
    EXPECT_THAT(opts.quality, Eq(9));
}

TEST_F(ArgsTest, ShortOptionsFail) {
    ShortOptions opts;
    fail("extra arguments found: \"extra\"", opts.callbacks(), "extra");
    fail("illegal option: -a", opts.callbacks(), "-a");
    fail("-t: argument required", opts.callbacks(), "-t");
    fail("illegal option: --t", opts.callbacks(), "--t");
    fail("illegal option: --t", opts.callbacks(), "--t=TEXT");

    fail("extra arguments found: \"extra\"", opts.callbacks(), "-kmg", "-iin", "-oout", "extra");
    fail("illegal option: -a", opts.callbacks(), "-kmg", "-iin", "-oout", "-a");
    fail("illegal option: --t", opts.callbacks(), "-kmg", "-iin", "-oout", "--t");

    opts.verbosity = std::numeric_limits<int>::max() - 1;
    pass(opts.callbacks(), "-v");
    fail("-v: integer overflow", opts.callbacks(), "-vv");
    fail("-v: integer overflow", opts.callbacks(), "-vvv");

    fail("-qx: invalid integer", opts.callbacks(), "-qx");
    fail("-q x: invalid integer", opts.callbacks(), "-q", "x");
    fail("-q -1: integer overflow", opts.callbacks(), "-q", "-1");
    fail("-q 65536: integer overflow", opts.callbacks(), "-q", "65536");
}

struct Greeter {
    Greeter() : _exclamation_point(true), _greeting("Hello"), _name("world"), _times(1) {}

    args::callbacks callbacks() {
        args::callbacks callbacks;
        callbacks.long_option = [this](
                pn::string_view opt, const args::callbacks::get_value_f& get_value) {
            if (opt == "normal") {
                _exclamation_point = false;
            } else if (opt == "exclamation-point") {
                _exclamation_point = true;
            } else if (opt == "hello") {
                _greeting = "Hello";
            } else if (opt == "ελληνικά") {
                _greeting = "Καλημέρα";
            } else if (opt == "日本語") {
                _greeting = "こんにちは";
            } else if (opt == "name") {
                _name = get_value().copy();
            } else if (opt == "again") {
                args::increment_option(&_times, 1);
            } else {
                return false;
            }
            return true;
        };
        return callbacks;
    }

    pn::string make_greeting() {
        pn::string      result;
        pn::file        f      = result.open("w");
        pn::string_view suffix = _exclamation_point ? "!" : ".";
        for (int i : range(_times)) {
            static_cast<void>(i);
            pn::format(f, "{0}, {1}{2}\n", _greeting, _name, suffix);
        }
        return result;
    }

  private:
    bool       _exclamation_point;
    pn::string _greeting;
    pn::string _name;
    int        _times;

    DISALLOW_COPY_AND_ASSIGN(Greeter);
};

TEST_F(ArgsTest, LongOptionsNone) {
    Greeter opts;
    pass(opts.callbacks());
    EXPECT_THAT(opts.make_greeting(), Eq("Hello, world!\n"));
}

TEST_F(ArgsTest, LongOptionsAll) {
    Greeter opts;
    pass(opts.callbacks(), "--again", "--ελληνικά", "--name=Ελένη", "--normal", "--again");
    EXPECT_THAT(
            opts.make_greeting(), Eq("Καλημέρα, Ελένη.\nΚαλημέρα, Ελένη.\nΚαλημέρα, Ελένη.\n"));
}

struct ArgumentsOnly {
    Optional<int32_t>    one;
    Optional<pn::string> two;
    vector<int32_t>      three;

    ArgumentsOnly() {}

    args::callbacks callbacks() {
        args::callbacks callbacks;
        callbacks.argument = [this](pn::string_view arg) {
            if (!one.has()) {
                int32_t value;
                args::integer_option(arg, &value);
                one.set(value);
            } else if (!two.has()) {
                two.set(arg.copy());
            } else {
                int32_t value;
                args::integer_option(arg, &value);
                three.push_back(value);
            }
            return true;
        };
        return callbacks;
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(ArgumentsOnly);
};

TEST_F(ArgsTest, ArgumentsEmpty) {
    ArgumentsOnly opts;
    pass(opts.callbacks());
    EXPECT_THAT(opts.one.has(), Eq(false));
    EXPECT_THAT(opts.two.has(), Eq(false));
    EXPECT_THAT(opts.three, IsEmpty());
}

TEST_F(ArgsTest, ArgumentsOne) {
    ArgumentsOnly opts;
    pass(opts.callbacks(), "1");
    EXPECT_THAT(*opts.one, Eq(1));
    EXPECT_THAT(opts.two.has(), Eq(false));
    EXPECT_THAT(opts.three, ElementsAre());
}

TEST_F(ArgsTest, ArgumentsTwo) {
    ArgumentsOnly opts;
    pass(opts.callbacks(), "1", "2");
    EXPECT_THAT(*opts.one, Eq(1));
    EXPECT_THAT(*opts.two, Eq("2"));
    EXPECT_THAT(opts.three, ElementsAre());
}

TEST_F(ArgsTest, ArgumentsThree) {
    ArgumentsOnly opts;
    pass(opts.callbacks(), "1", "2", "3");
    EXPECT_THAT(*opts.one, Eq(1));
    EXPECT_THAT(*opts.two, Eq("2"));
    EXPECT_THAT(opts.three, ElementsAre(3));
}

TEST_F(ArgsTest, ArgumentsAll) {
    ArgumentsOnly opts;
    pass(opts.callbacks(), "1", "2", "3", "4", "5");
    EXPECT_THAT(*opts.one, Eq(1));
    EXPECT_THAT(*opts.two, Eq("2"));
    EXPECT_THAT(opts.three, ElementsAre(3, 4, 5));
}

TEST_F(ArgsTest, ArgumentsDash) {
    ArgumentsOnly opts;
    pass(opts.callbacks(), "1", "-", "3");
    EXPECT_THAT(*opts.one, Eq(1));
    EXPECT_THAT(*opts.two, Eq("-"));
    EXPECT_THAT(opts.three, ElementsAre(3));
}

TEST_F(ArgsTest, ArgumentsDashDash) {
    ArgumentsOnly opts;
    pass(opts.callbacks(), "1", "--", "--2", "-3");
    EXPECT_THAT(*opts.one, Eq(1));
    EXPECT_THAT(*opts.two, Eq("--2"));
    EXPECT_THAT(opts.three, ElementsAre(-3));
}

TEST_F(ArgsTest, ArgumentsFail) {
    ArgumentsOnly opts;
    fail("illegal option: -s", opts.callbacks(), "-s");
    fail("illegal option: --long", opts.callbacks(), "--long");
    fail("x: invalid integer", opts.callbacks(), "1", "2", "x");
    fail("x: invalid integer", opts.callbacks(), "1", "2", "3", "4", "x");
    fail("2147483648: integer overflow", opts.callbacks(), "1", "2", "3", "4", "2147483648");
}

class CutTool {
  public:
    CutTool() : _limit(std::numeric_limits<int64_t>::max()), _delimiter("\t") {}

    args::callbacks callbacks() {
        args::callbacks callbacks;
        callbacks.short_option = [this](
                pn::rune opt, const args::callbacks::get_value_f& get_value) {
            switch (opt.value()) {
                case 'l': args::integer_option(get_value(), &_limit); return true;
                case 'd': _delimiter = get_value().copy(); return true;
                default: return false;
            }
        };
        callbacks.long_option = [this, callbacks](
                pn::string_view opt, const args::callbacks::get_value_f& get_value) {
            if (opt == "limit") {
                return callbacks.short_option(pn::rune{'d'}, get_value);
            } else if (opt == "delimiter") {
                return callbacks.short_option(pn::rune{'d'}, get_value);
            } else {
                return false;
            }
        };
        callbacks.argument = [this](pn::string_view arg) {
            if (!_string.has()) {
                _string.set(arg.copy());
            } else {
                return false;
            }
            return true;
        };
        return callbacks;
    }

    vector<pn::string_view>& cut() {
        pn::string_view remainder = *_string;
        bool            next      = true;
        int             splits    = 1;
        while (next) {
            pn::string_view token;
            next = pn::partition(token, _delimiter, remainder);
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
    sfz::Optional<pn::string> _string;
    int64_t                   _limit;
    pn::string                _delimiter;

    vector<pn::string_view> _result;

    DISALLOW_COPY_AND_ASSIGN(CutTool);
};

TEST_F(ArgsTest, CutSimple) {
    CutTool opts;
    pass(opts.callbacks(), "1\t2\t3");
    ASSERT_THAT(opts.cut(), ElementsAre("1", "2", "3"));
}

TEST_F(ArgsTest, CutShort) {
    CutTool opts;
    pass(opts.callbacks(), "doo-wop", "-d-");
    ASSERT_THAT(opts.cut(), ElementsAre("doo", "wop"));
}

TEST_F(ArgsTest, CutLong) {
    CutTool opts;
    pass(opts.callbacks(), "--delimiter=an", "A man, a plan, a canal, Panama", "-l4");
    ASSERT_THAT(opts.cut(), ElementsAre("A m", ", a pl", ", a c", "al, Panama"));
}

struct Calculator {
    Calculator() { reset(); }

    void reset() {
        _x.clear();
        _y.clear();
        _op           = 0;
        _int_division = false;
    }

    args::callbacks callbacks() {
        args::callbacks callbacks;
        callbacks.long_option = [this](
                pn::string_view opt, const args::callbacks::get_value_f& get_value) {
            switch (_op) {
                case 'a': return false;
                case '+': return false;
                case '/':
                    if (opt == "int") {
                        _int_division = true;
                    } else {
                        return false;
                    }
                    return true;
            }
            return false;
        };
        callbacks.argument = [this](pn::string_view arg) {
            if (!_x.has()) {
                args::float_option(arg, &_x);
            } else if (_op == 0) {
                if (arg == "abs") {
                    _op = 'a';
                } else if (arg == "plus") {
                    _op = '+';
                } else if (arg == "div") {
                    _op = '/';
                } else {
                    throw std::runtime_error("unknown command");
                }
            } else if (!_y.has()) {
                switch (_op) {
                    case 'a': return false;
                    case '+': args::float_option(arg, &_y); return true;
                    case '/': args::float_option(arg, &_y); return true;
                }
            } else {
                return false;
            }
            return true;
        };
        return callbacks;
    }

    double value() const {
        double result;
        switch (_op) {
            case 'a': result = fabs(*_x); break;
            case '+': result = *_x + *_y; break;
            case '/':
                if (_int_division) {
                    result = int64_t(*_x) / int64_t(*_y);
                } else {
                    result = *_x / *_y;
                }
                break;
            default: return *_x;
        }
        return result;
    }

  private:
    sfz::Optional<double> _x;
    sfz::Optional<double> _y;
    char                  _op;
    bool                  _int_division;

    DISALLOW_COPY_AND_ASSIGN(Calculator);
};

TEST_F(ArgsTest, CalculatorMinimal) {
    Calculator calc;
    pass(calc.callbacks(), "5", "abs");
    EXPECT_THAT(calc.value(), Eq(5));

    calc.reset();
    pass(calc.callbacks(), "--", "-4", "abs");
    EXPECT_THAT(calc.value(), Eq(4));

    calc.reset();
    pass(calc.callbacks(), "1", "plus", "2");
    EXPECT_THAT(calc.value(), Eq(3));

    calc.reset();
    pass(calc.callbacks(), "4", "div", "2");
    EXPECT_THAT(calc.value(), Eq(2));
}

TEST_F(ArgsTest, CalculatorOptions) {
    Calculator calc;
    pass(calc.callbacks(), "5", "div", "--int", "2");
    EXPECT_THAT(calc.value(), Eq(2));

    calc.reset();
    pass(calc.callbacks(), "5", "div", "2", "--int");
    EXPECT_THAT(calc.value(), Eq(2));

    calc.reset();
    pass(calc.callbacks(), "5", "div", "2");
    EXPECT_THAT(calc.value(), Eq(2.5));
}

}  // namespace
}  // namespace sfz
