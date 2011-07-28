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
    State(const Parser& spec):
            _spec(spec),
            _expecting_value(false),
            _argument(_spec._argument_specs.begin()) { }

    void parse_args(const vector<StringSlice>& args) {
        bool saw_dash_dash = false;
        SFZ_FOREACH(StringSlice token, args, {
            if (_expecting_value) {
                process_short_option(_option_expected, token);
                _expecting_value = false;
            } else if (saw_dash_dash) {
                process_argument(token);
            } else if (token == "--") {
                saw_dash_dash = true;
            } else if (starts_with(token, "--")) {
                parse_long_option(token);
            } else if (token == "-") {
                process_argument(token);
            } else if (starts_with(token, "-")) {
                parse_short_option(token.slice(1));
            } else {
                process_argument(token);
            }
        });
        if (_expecting_value) {
            String string_option(1, _option_expected);
            throw Exception(format("Expecting value for option \"-{0}\"", string_option));
        }
        while (_argument != _spec._argument_specs.end()) {
            if ((*_argument)->_min_args > 0) {
                throw Exception("Missing required argument");
            }
            ++_argument;
        }
    }

    void parse_long_option(StringSlice token) {
        StringSlice option;
        if (partition(option, "=", token)) {
            StringSlice value = token;
            if (!long_option_exists(option)) {
                throw Exception(format("no such long option {0}", option));
            } else if (!long_option_takes_value(option)) {
                throw Exception(format("long option {0} doesn't take value", option));
            } else {
                process_long_option(option, value);
            }
        } else {
            if (!long_option_exists(option)) {
                throw Exception(format("no such long option {0}", option));
            } else if (long_option_takes_value(option)) {
                throw Exception(format("long option --{0} takes value", option));
            } else {
                process_long_option(option);
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

    void process_long_option(StringSlice option) {
        long_option(option)._action.process();
    }

    void process_long_option(StringSlice option, StringSlice value) {
        long_option(option)._action.process(value);
    }

    void parse_short_option(StringSlice token) {
        Rune option = token.at(0);
        StringSlice remainder = token.slice(1);
        if (!short_option_exists(option)) {
            throw Exception(format("no such short option -{0}", option));
        } else if (short_option_takes_value(option)) {
            if (remainder.empty()) {
                _option_expected = option;
                _expecting_value = true;
            } else {
                process_short_option(option, remainder);
            }
        } else {
            process_short_option(option);
            if (!remainder.empty()) {
                parse_short_option(remainder);
            }
        }
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

    void process_short_option(Rune option) {
        short_option(option)._action.process();
    }

    void process_short_option(Rune option, StringSlice value) {
        short_option(option)._action.process(value);
    }

    void process_argument(StringSlice value) {
        if (_argument == _spec._argument_specs.end()) {
            throw Exception("too many arguments");
        }
        (*_argument)->_action.process(value);
        int& nargs = _nargs[_argument->get()];
        ++nargs;
        if (nargs == (*_argument)->_max_args) {
            ++_argument;
        }
    }

  private:
    const Parser& _spec;

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

void Parser::parse_args(const vector<StringSlice>& args) const {
    State(*this).parse_args(args);
}

void Parser::parse_args(int argc, const char* const* argv) const {
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
    parse_args(args);
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
void Action::process() const { return _impl->process(); }
void Action::process(StringSlice value) const { return _impl->process(value); }

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

void store_argument(bool& to, StringSlice value) {
    if (value == "true") {
        to = true;
    } else if (value == "false") {
        to = false;
    } else {
        throw Exception("bad boolean");
    }
}

template <typename T>
void store_integral_argument(T& to, StringSlice value) {
    if (!string_to_int(value, to, 10)) {
        throw Exception("bad integer");
    }
}

void store_argument(int8_t& to, StringSlice value) { store_integral_argument(to, value); }
void store_argument(uint8_t& to, StringSlice value) { store_integral_argument(to, value); }
void store_argument(int16_t& to, StringSlice value) { store_integral_argument(to, value); }
void store_argument(uint16_t& to, StringSlice value) { store_integral_argument(to, value); }
void store_argument(int32_t& to, StringSlice value) { store_integral_argument(to, value); }
void store_argument(uint32_t& to, StringSlice value) { store_integral_argument(to, value); }
void store_argument(int64_t& to, StringSlice value) { store_integral_argument(to, value); }
void store_argument(uint64_t& to, StringSlice value) { store_integral_argument(to, value); }

void print_to(PrintTarget out, ParserUsage usage) {
    usage.parser.print_usage_to(out);
}

}  // namespace args
}  // namespace sfz
