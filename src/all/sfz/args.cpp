// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/args.hpp>

#include <stdlib.h>
#include <sfz/encoding.hpp>
#include <sfz/exception.hpp>
#include <sfz/foreach.hpp>
#include <sfz/format.hpp>
#include <sfz/string-utils.hpp>

using std::map;
using std::pair;
using std::vector;

namespace sfz {
namespace args {

namespace {

bool starts_with(StringSlice string, StringSlice prefix) {
    return (string.size() >= prefix.size())
        && (string.slice(0, prefix.size()) == prefix);
}

bool is_valid_short_option(StringSlice s) {
    return (s.size() == 2) && (s.at(0) == '-') && (s.at(1) != '-') && (s.at(1) != '=');
}

bool is_valid_long_option(StringSlice s) {
    return (s.size() > 2) && (s.at(0) == '-') && (s.at(1) == '-')
        && (s.find('=') == StringSlice::npos);
}

}  // namespace

class Parser::State {
  public:
    State(const Parser& spec, PrintTarget error):
            _spec(spec),
            _error(error),
            _expecting_value(false),
            _argument(_spec._argument_specs.begin()) { }

    bool parse_args(const vector<StringSlice>& args) {
        bool saw_dash_dash = false;
        SFZ_FOREACH(StringSlice token, args, {
            if (_expecting_value) {
                if (!process_short_option(_option_expected, token)) {
                    return false;
                }
                _expecting_value = false;
            } else if (saw_dash_dash) {
                if (!process_argument(token)) {
                    return false;
                }
            } else if (token == "--") {
                saw_dash_dash = true;
            } else if (starts_with(token, "--")) {
                if (!parse_long_option(token)) {
                    return false;
                }
            } else if (token == "-") {
                if (!process_argument(token)) {
                    return false;
                }
            } else if (starts_with(token, "-")) {
                if (!parse_short_option(token.slice(1))) {
                    return false;
                }
            } else {
                if (!process_argument(token)) {
                    return false;
                }
            }
        });
        if (_expecting_value) {
            String string_option(1, _option_expected);
            print(_error, format("option requires an argument: -{0}", string_option));
            return false;
        }
        while (_argument != _spec._argument_specs.end()) {
            if ((*_argument)->_min_args > 0) {
                print(_error, "not enough arguments");
                return false;
            }
            ++_argument;
        }
        return true;
    }

    bool parse_long_option(StringSlice token) {
        StringSlice option;
        if (partition(option, "=", token)) {
            StringSlice value = token;
            if (!long_option_exists(option)) {
                print(_error, format("illegal option: {0}", option));
                return false;
            } else if (!long_option_takes_value(option)) {
                print(_error, format("option does not allow an argument: {0}", option));
                return false;
            } else {
                return process_long_option(option, value);
            }
        } else {
            if (!long_option_exists(option)) {
                print(_error, format("illegal option: {0}", option));
                return false;
            } else if (long_option_takes_value(option)) {
                print(_error, format("option requires an argument: {0}", option));
                return false;
            } else {
                return process_long_option(option);
            }
        }
    }

    bool long_option_exists(StringSlice option) {
        return _spec._long_options_by_name.find(option) != _spec._long_options_by_name.end();
    }

    const Argument& long_option(StringSlice option) {
        return *_spec._long_options_by_name.find(option)->second;
    }

    bool long_option_takes_value(StringSlice option) {
        return long_option(option)._action.takes_value();
    }

    bool process_long_option(StringSlice option) {
        if (!long_option(option)._action.process(_error)) {
            print(_error, format(": {0}", option));
            return false;
        }
        return true;
    }

    bool process_long_option(StringSlice option, StringSlice value) {
        if (!long_option(option)._action.process(value, _error)) {
            print(_error, format(": {0}={1}", option, quote(value)));
            return false;
        }
        return true;
    }

    bool parse_short_option(StringSlice token) {
        Rune option = token.at(0);
        StringSlice remainder = token.slice(1);
        if (!short_option_exists(option)) {
            print(_error, format("illegal option: -{0}", token.slice(0, 1)));
            return false;
        } else if (short_option_takes_value(option)) {
            if (remainder.empty()) {
                _option_expected = option;
                _expecting_value = true;
            } else {
                return process_short_option(option, remainder);
            }
        } else {
            if (!process_short_option(option)) {
                return false;
            }
            if (!remainder.empty()) {
                return parse_short_option(remainder);
            }
        }
        return true;
    }

    bool short_option_exists(Rune option) {
        return _spec._short_options_by_name.find(option) != _spec._short_options_by_name.end();
    }

    const Argument& short_option(Rune option) {
        return *_spec._short_options_by_name.find(option)->second;
    }

    bool short_option_takes_value(Rune option) {
        return short_option(option)._action.takes_value();
    }

    bool process_short_option(Rune option) {
        if (!short_option(option)._action.process(_error)) {
            String s(1, option);
            print(_error, format(": -{0}", s));
            return false;
        }
        return true;
    }

    bool process_short_option(Rune option, StringSlice value) {
        if (!short_option(option)._action.process(value, _error)) {
            String s(1, option);
            print(_error, format(": -{0} {1}", s, quote(value)));
            return false;
        }
        return true;
    }

    bool process_argument(StringSlice value) {
        if (_argument == _spec._argument_specs.end()) {
            print(_error, "too many arguments");
            return false;
        }
        if (!(*_argument)->_action.process(value, _error)) {
            return false;
        }
        int& nargs = _nargs[_argument->get()];
        ++nargs;
        if (nargs == (*_argument)->_max_args) {
            ++_argument;
        }
        return true;
    }

  private:
    const Parser& _spec;
    PrintTarget _error;

    Rune _option_expected;
    bool _expecting_value;

    vector<linked_ptr<Argument> >::const_iterator _argument;
    map<Argument*, int> _nargs;

    DISALLOW_COPY_AND_ASSIGN(State);
};

Parser::Parser(PrintItem program_name, PrintItem description):
        _program_name(program_name),
        _description(description) { }

Parser::Parser(const char* program_name, PrintItem description):
        _program_name(utf8::decode(program_name)),
        _description(description) { }

Argument& Parser::add_argument(PrintItem name, Action action) {
    String printed_name(name);
    if (printed_name.empty()) {
        throw Exception("invalid argument name");
    }
    if (printed_name.at(0) == '-') {
        if (is_valid_short_option(printed_name)) {
            linked_ptr<Argument> arg(new Argument(true, action));
            _short_options_by_name[printed_name.at(1)] = arg;
            arg->_metavar.assign(printed_name.slice(1));
            upper(arg->_metavar);
            return *arg;
        } else if (is_valid_long_option(printed_name)) {
            linked_ptr<Argument> arg(new Argument(true, action));
            _long_options_by_name[printed_name] = arg;
            arg->_metavar.assign(printed_name.slice(2));
            upper(arg->_metavar);
            return *arg;
        } else {
            throw Exception("invalid argument name");
        }
    } else {
        linked_ptr<Argument> arg(new Argument(false, action));
        _argument_specs.push_back(arg);
        arg->_metavar.assign(printed_name);
        return *arg;
    }
}

Argument& Parser::add_argument(PrintItem short_name, PrintItem long_name, Action action) {
    String printed_short_name(short_name);
    String printed_long_name(long_name);
    if (!is_valid_short_option(printed_short_name)) {
        throw Exception("invalid argument name");
    } else if (!is_valid_long_option(printed_long_name)) {
        throw Exception("invalid argument name");
    }
    linked_ptr<Argument> arg(new Argument(true, action));
    _short_options_by_name[printed_short_name.at(1)] = arg;
    _long_options_by_name[printed_long_name] = arg;
    arg->_metavar.assign(printed_long_name.slice(2));
    upper(arg->_metavar);
    return *arg;
}

bool Parser::parse_args(const vector<StringSlice>& args, PrintTarget error) const {
    return State(*this, error).parse_args(args);
}

bool Parser::parse_args(int argc, const char* const* argv, PrintTarget error) const {
    String storage;
    vector<size_t> cuts(argc, 0);
    SFZ_FOREACH(size_t i, range(argc), {
        storage.append(utf8::decode(argv[i]));
        cuts[i] = storage.size();
    });
    size_t begin = 0;
    vector<StringSlice> args(argc, "");
    SFZ_FOREACH(size_t i, range(argc), {
        args[i] = storage.slice(begin, cuts[i] - begin);
        begin = cuts[i];
    });
    return parse_args(args, error);
}

const String& Parser::program_name() const {
    return _program_name;
}

ParserUsage Parser::usage() const {
    ParserUsage result = {*this};
    return result;
}

void Parser::print_usage_to(PrintTarget out) const {
    typedef pair<Rune, linked_ptr<Argument> > ShortArg;
    typedef pair<StringSlice, linked_ptr<Argument> > LongArg;

    print(out, _program_name);

    bool has_argless_options = false;
    SFZ_FOREACH(const ShortArg& arg, _short_options_by_name, {
        if (!arg.second->_action.takes_value()) {
            if (!has_argless_options) {
                print(out, " [-");
                has_argless_options = true;
            }
            out.push(1, arg.first);
        }
    });
    if (has_argless_options) {
        print(out, "]");
    }

    SFZ_FOREACH(const ShortArg& arg, _short_options_by_name, {
        if (arg.second->_action.takes_value()) {
            String rune(1, arg.first);
            print(out, format(" [-{0} {1}]", rune, arg.second->_metavar));
        }
    });

    SFZ_FOREACH(const LongArg& arg, _long_options_by_name, {
        if (arg.second->_action.takes_value()) {
            print(out, format(" [{0}={1}]", arg.first, arg.second->_metavar));
        } else {
            print(out, format(" [{0}]", arg.first));
        }
    });

    int nesting = 0;
    SFZ_FOREACH(const linked_ptr<Argument>& arg, _argument_specs, {
        SFZ_FOREACH(int i, range(arg->_min_args), {
            print(out, format(" {0}", arg->_metavar));
        });
        if (arg->_max_args == std::numeric_limits<int>::max()) {
            print(out, format(" [{0}...]", arg->_metavar));
        } else {
            SFZ_FOREACH(int i, range(arg->_max_args - arg->_min_args), {
                ++nesting;
                print(out, format(" [{0}", arg->_metavar));
            });
        }
    });
    out.push(nesting, ']');
}

Action::Action(const linked_ptr<Impl>& impl): _impl(impl) { }
Action::Action(const Action& other): _impl(other._impl) { }
Action& Action::operator=(const Action& other) { _impl = other._impl; return *this; }
Action::~Action() { }
bool Action::takes_value() const { return _impl->takes_value(); }
bool Action::process(PrintTarget error) const { return _impl->process(error); }
bool Action::process(StringSlice value, PrintTarget error) const { return _impl->process(value, error); }

Argument::Argument(bool option, Action action):
        _option(option),
        _action(action),
        _min_args(0),
        _max_args(1) { }

Argument& Argument::help(PrintItem s) {
    _help.assign(s);
    return *this;
}

Argument& Argument::metavar(PrintItem s) {
    _metavar.assign(s);
    return *this;
}

Argument& Argument::required() {
    if (_option) {
        throw Exception("called required() on an option");
    }
    return nargs(1);
}

Argument& Argument::nargs(int n) {
    if (_option) {
        throw Exception("called nargs() on an option");
    }
    return min_args(n).max_args(n);
}

Argument& Argument::min_args(int n) {
    if (_option) {
        throw Exception("called min_args() on an option");
    }
    _min_args = n;
    return *this;
}

Argument& Argument::max_args(int n) {
    if (_option) {
        throw Exception("called max_args() on an option");
    }
    _max_args = n;
    return *this;
}

bool store_argument(bool& to, StringSlice value, PrintTarget error) {
    if (value == "true") {
        to = true;
        return true;
    } else if (value == "false") {
        to = false;
        return true;
    } else {
        print(error, "must be true or false");
        return false;
    }
}

template <typename T>
bool store_integral_argument(T& to, StringSlice value, PrintTarget error) {
    StringToIntResult result = string_to_int(value, to);
    switch (result.failure) {
      case StringToIntResult::NONE:
        return true;
      case StringToIntResult::INVALID_LITERAL:
        print(error, "invalid integer");
        return false;
      case StringToIntResult::INTEGER_OVERFLOW:
        print(error, "integer overflow");
        return false;
    }
    return false;
}

bool store_argument(int8_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(uint8_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(int16_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(uint16_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(int32_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(uint32_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(int64_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(uint64_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }

void print_to(PrintTarget out, ParserUsage usage) {
    usage.parser.print_usage_to(out);
}

}  // namespace args
}  // namespace sfz
