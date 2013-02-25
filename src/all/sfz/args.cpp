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
#include <sfz/io.hpp>
#include <sfz/string-utils.hpp>

using std::map;
using std::pair;
using std::vector;
using std::shared_ptr;

namespace sfz {
namespace args {

namespace {

struct Indented {
    PrintTarget out;
    int32_t indent;
    bool line_started;

    void push(StringSlice string) {
        StringSlice line;
        while (partition(line, "\n", string)) {
            if (!line_started && !line.empty()) {
                out.push(indent, ' ');
            }
            out.push(line);
            out.push(1, '\n');
            line_started = false;
        }
        if (!line_started && !line.empty()) {
            out.push(indent, ' ');
            line_started = true;
        }
        out.push(line);
        line_started = !line.empty();
    }

    void push(size_t num, Rune rune) {
        out.push(num, rune);
        if (rune == '\n') {
            line_started = false;
        }
    }
};

struct PrintableRune { Rune r; };
PrintableRune rune(Rune r) {
    PrintableRune rune = {r};
    return rune;
}
void print_to(PrintTarget out, PrintableRune rune) {
    out.push(1, rune.r);
}

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
            _arguments_saturated(false),
            _argument(_spec._argument_specs.begin()) { }

    bool parse_args(
            // TODO(sfiera): use an array slice.
            vector<StringSlice>::const_iterator begin,
            vector<StringSlice>::const_iterator end) {
        if (!_spec._action.process(_error)) {
            return false;
        }
        bool saw_dash_dash = false;
        for ( ; begin != end; ++begin) {
            const StringSlice token = *begin;
            if (_arguments_saturated) {
                print(_error, format(" {0}", quote(token)));
            } else if (_expecting_value) {
                if (!process_short_option(_option_expected, token)) {
                    return false;
                }
                _expecting_value = false;
            } else if (!saw_dash_dash && (token == "--")) {
                saw_dash_dash = true;
            } else if (!saw_dash_dash && starts_with(token, "--")) {
                if (!parse_long_option(token)) {
                    return false;
                }
            } else if (!saw_dash_dash && starts_with(token, "-") && (token != "-")) {
                if (!parse_short_option(token.slice(1))) {
                    return false;
                }
            } else {
                if (!args_remaining() && _spec.has_subparsers()) {
                    return process_subparser(token, ++begin, end);
                }
                if (!process_argument(token)) {
                    return false;
                }
            }
        }
        if (_arguments_saturated) {
            return false;
        } else if (_expecting_value) {
            String string_option(1, _option_expected);
            print(_error, format("option -{0}: argument required", string_option));
            return false;
        }
        while (args_remaining()) {
            if ((*_argument)->_min_args > 0) {
                print(_error, "too few arguments");
                return false;
            }
            ++_argument;
        }
        return true;
    }

    bool args_remaining() {
        return _argument != _spec._argument_specs.end();
    }

    bool parse_long_option(StringSlice token) {
        StringSlice option;
        if (partition(option, "=", token)) {
            StringSlice value = token;
            if (!_spec.has_long_option(option)) {
                print(_error, format("illegal option: {0}", option));
                return false;
            } else if (!_spec.long_option_takes_value(option)) {
                print(_error, format("option does not allow an argument: {0}", option));
                return false;
            } else {
                return process_long_option(option, value);
            }
        } else {
            if (!_spec.has_long_option(option)) {
                print(_error, format("illegal option: {0}", option));
                return false;
            } else if (_spec.long_option_takes_value(option)) {
                print(_error, format("option requires an argument: {0}", option));
                return false;
            } else {
                return process_long_option(option);
            }
        }
    }

    bool process_long_option(StringSlice option) {
        if (!_spec.long_option(option)._action.process(_action_error)) {
            print(_error, format("option {0}: {1}", option, _action_error));
            return false;
        }
        return true;
    }

    bool process_long_option(StringSlice option, StringSlice value) {
        if (!_spec.long_option(option)._action.process(value, _action_error)) {
            print(_error, format("option {0}: {1}: {2}", option, _action_error, quote(value)));
            return false;
        }
        return true;
    }

    bool parse_short_option(StringSlice token) {
        Rune option = token.at(0);
        StringSlice remainder = token.slice(1);
        if (!_spec.has_short_option(option)) {
            print(_error, format("illegal option: -{0}", token.slice(0, 1)));
            return false;
        } else if (_spec.short_option_takes_value(option)) {
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

    bool process_short_option(Rune option) {
        if (!_spec.short_option(option)._action.process(_action_error)) {
            String s(1, option);
            print(_error, format("option -{0}: {1}", s, _action_error));
            return false;
        }
        return true;
    }

    bool process_short_option(Rune option, StringSlice value) {
        if (!_spec.short_option(option)._action.process(value, _action_error)) {
            String s(1, option);
            print(_error, format("option -{0}: {1}: {2}", s, _action_error, quote(value)));
            return false;
        }
        return true;
    }

    bool process_argument(StringSlice value) {
        if (_argument == _spec._argument_specs.end()) {
            print(_error, format("extra arguments found: {0}", quote(value)));
            _arguments_saturated = true;
            return true;
        }
        if (!(*_argument)->_action.process(value, _action_error)) {
            print(_error, format("argument {0}: {1}: {2}",
                        (*_argument)->_metavar, _action_error, quote(value)));
            return false;
        }
        int& nargs = _nargs[_argument->get()];
        ++nargs;
        if (nargs == (*_argument)->_max_args) {
            ++_argument;
        }
        return true;
    }

    bool process_subparser(
            StringSlice token,
            vector<StringSlice>::const_iterator begin,
            vector<StringSlice>::const_iterator end) {
        if (_spec.has_subparser(token)) {
            State substate(_spec.subparser(token), _error);
            return substate.parse_args(begin, end);
        } else {
            print(_error, format("{0} is not a {1} command", quote(token), _spec.name()));
            return false;
        }
    }

  private:
    const Parser& _spec;
    PrintTarget _error;
    String _action_error;

    Rune _option_expected;
    bool _expecting_value;
    bool _arguments_saturated;

    vector<shared_ptr<Argument> >::const_iterator _argument;
    map<Argument*, int> _nargs;

    DISALLOW_COPY_AND_ASSIGN(State);
};

Parser::Parser(PrintItem program_name, PrintItem description, Action action):
        _parent(NULL),
        _name(program_name),
        _description(description),
        _action(action) { }

Parser::Parser(const char* program_name, PrintItem description, Action action):
        _parent(NULL),
        _name(utf8::decode(program_name)),
        _description(description),
        _action(action) { }

Parser::Parser(Parser* const parent, PrintItem name, PrintItem description, Action action):
        _parent(parent),
        _name(name),
        _description(description),
        _action(action) { }

Argument& Parser::add_argument(PrintItem name, Action action) {
    String printed_name(name);
    if (printed_name.empty()) {
        throw Exception("invalid argument name");
    }
    if (printed_name.at(0) == '-') {
        if (is_valid_short_option(printed_name)) {
            shared_ptr<Argument> arg(
                    new Argument(Argument::SHORT_OPTION, printed_name, "", action));
            _option_specs.push_back(arg);
            _short_options_by_name[printed_name.at(1)] = arg;
            arg->_metavar.assign(printed_name.slice(1));
            upper(arg->_metavar);
            return *arg;
        } else if (is_valid_long_option(printed_name)) {
            shared_ptr<Argument> arg(
                    new Argument(Argument::LONG_OPTION, "", printed_name, action));
            _option_specs.push_back(arg);
            _long_options_by_name[printed_name] = arg;
            arg->_metavar.assign(printed_name.slice(2));
            upper(arg->_metavar);
            return *arg;
        } else {
            throw Exception("invalid argument name");
        }
    } else {
        shared_ptr<Argument> arg(
                new Argument(Argument::ARGUMENT, "", "", action));
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
    shared_ptr<Argument> arg(
            new Argument(Argument::BOTH_OPTION, printed_short_name, printed_long_name, action));
    _option_specs.push_back(arg);
    _short_options_by_name[printed_short_name.at(1)] = arg;
    _long_options_by_name[printed_long_name] = arg;
    arg->_metavar.assign(printed_long_name.slice(2));
    upper(arg->_metavar);
    return *arg;
}

Parser& Parser::add_subparser(PrintItem name, PrintItem description, Action action) {
    String printed_name(name);
    if (printed_name.empty() || (printed_name.at(0) == '-')) {
        throw Exception("invalid subcommand name");
    }
    if (has_subparser(printed_name)) {
        throw Exception("duplicate command name");
    }
    shared_ptr<Parser> subparser(new Parser(this, printed_name, description, action));
    _subparsers.push_back(subparser);
    _subparsers_by_name[printed_name] = subparser;
    return *subparser;
}

bool Parser::parse_args(const vector<StringSlice>& args, PrintTarget error) const {
    return State(*this, error).parse_args(args.begin(), args.end());
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

const String& Parser::name() const {
    return _name;
}

ParserUsage Parser::usage() const {
    ParserUsage result = {*this};
    return result;
}

ParserHelp Parser::help() const {
    ParserHelp result = {*this};
    return result;
}

void Parser::print_usage_to(PrintTarget out) const {
    typedef pair<Rune, shared_ptr<Argument> > ShortArg;
    typedef pair<StringSlice, shared_ptr<Argument> > LongArg;

    print(out, _name);

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
            print(out, format(" [-{0} {1}]", rune(arg.first), arg.second->_metavar));
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
    SFZ_FOREACH(const shared_ptr<Argument>& arg, _argument_specs, {
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

    if (has_subparsers()) {
        print(out, " [command]");
    }
}

void Parser::print_help_to(PrintTarget out) const {
    print(out, format("usage: {0}\n", usage()));
    Indented body = {out, 2};
    print(body, format("\n{0}\n", _description));

    if (!_argument_specs.empty()) {
        print(body, "\narguments:\n");
        SFZ_FOREACH(const shared_ptr<Argument>& arg, _argument_specs, {
            print(body, format("  {0}", arg->_metavar));
            if (!arg->_help.empty()) {
                int padding = 20 - arg->_metavar.size();
                if (padding <= 0) {
                    body.push(1, '\n');
                    body.push(22, ' ');
                } else {
                    body.push(padding, ' ');
                }
                print(body, arg->_help);
            }
            print(body, "\n");
        });
    }

    if (!_option_specs.empty()) {
        print(body, "\noptions:\n");
        SFZ_FOREACH(const shared_ptr<Argument>& arg, _option_specs, {
            int padding = 22;
            switch (arg->_type) {
              case Argument::SHORT_OPTION:
                if (arg->_action.takes_value()) {
                    print(body, format("  {0} {1}", arg->_short_option_name, arg->_metavar));
                    padding -= 3 + arg->_short_option_name.size() + arg->_metavar.size();
                } else {
                    print(body, format("  {0}", arg->_short_option_name));
                    padding -= 2 + arg->_short_option_name.size();
                }
                break;

              case Argument::LONG_OPTION:
                if (arg->_action.takes_value()) {
                    print(body, format("      {0}={1}", arg->_long_option_name, arg->_metavar));
                    padding -= 7 + arg->_long_option_name.size() + arg->_metavar.size();
                } else {
                    print(body, format("      {0}", arg->_long_option_name));
                    padding -= 6 + arg->_long_option_name.size();
                }
                break;

              case Argument::BOTH_OPTION:
                if (arg->_action.takes_value()) {
                    print(body, format("  {0}, {1}={2}", arg->_short_option_name,
                            arg->_long_option_name, arg->_metavar));
                    padding -= 5 + arg->_short_option_name.size() + arg->_long_option_name.size() +
                        arg->_metavar.size();
                } else {
                    print(body, format("  {0}, {1}", arg->_short_option_name,
                                arg->_long_option_name));
                    padding -= 4 + arg->_short_option_name.size() + arg->_long_option_name.size();
                }
                break;

              default:
                break;
            }

            if (!arg->_help.empty()) {
                if (padding <= 0) {
                    body.push(1, '\n');
                    body.push(22, ' ');
                } else {
                    body.push(padding, ' ');
                }
                print(body, arg->_help);
            }
            print(body, "\n");
        });
    }

    if (has_subparsers()) {
        print(body, "\ncommands:\n");
        SFZ_FOREACH(const shared_ptr<Parser>& arg, _subparsers, {
            String usage(arg->usage());
            print(body, format("  {0}", usage));
            if (!arg->_description.empty()) {
                int padding = 20 - usage.size();
                if (padding <= 0) {
                    body.push(1, '\n');
                    body.push(22, ' ');
                } else {
                    body.push(padding, ' ');
                }
                print(body, arg->_description);
            }
            print(body, "\n");
        });
    }
}

bool Parser::has_long_option(StringSlice option) const {
    return (_long_options_by_name.find(option) != _long_options_by_name.end())
        || ((_parent != NULL) && (_parent->has_long_option(option)));
}

const Argument& Parser::long_option(StringSlice option) const {
    if (_long_options_by_name.find(option) != _long_options_by_name.end()) {
        return *_long_options_by_name.find(option)->second;
    } else {
        return _parent->long_option(option);
    }
}

bool Parser::long_option_takes_value(StringSlice option) const {
    return long_option(option)._action.takes_value();
}

bool Parser::has_short_option(Rune option) const {
    return (_short_options_by_name.find(option) != _short_options_by_name.end())
        || ((_parent != NULL) && (_parent->has_short_option(option)));
}

const Argument& Parser::short_option(Rune option) const {
    if (_short_options_by_name.find(option) != _short_options_by_name.end()) {
        return *_short_options_by_name.find(option)->second;
    } else {
        return _parent->short_option(option);
    }
}

bool Parser::short_option_takes_value(Rune option) const {
    return short_option(option)._action.takes_value();
}

bool Parser::has_subparsers() const {
    return !_subparsers.empty();
}

bool Parser::has_subparser(StringSlice name) const {
    return _subparsers_by_name.find(name) != _subparsers_by_name.end();
}

const Parser& Parser::subparser(StringSlice name) const {
    return *_subparsers_by_name.find(name)->second;
}

Action::Action(const shared_ptr<Impl>& impl): _impl(impl) { }
Action::Action(const Action& other): _impl(other._impl) { }
Action& Action::operator=(const Action& other) { _impl = other._impl; return *this; }
Action::~Action() { }

bool Action::takes_value() const {
    return (_impl.get() != NULL) && _impl->takes_value();
}

bool Action::process(PrintTarget error) const {
    return (_impl.get() == NULL) || _impl->process(error);
}

bool Action::process(StringSlice value, PrintTarget error) const {
    return _impl->process(value, error);
}

Argument::Argument(
        Type type, const StringSlice& short_option_name,
        const StringSlice& long_option_name, Action action):
        _type(type),
        _short_option_name(short_option_name),
        _long_option_name(long_option_name),
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
    if (_type != ARGUMENT) {
        throw Exception("called required() on an option");
    }
    return nargs(1);
}

Argument& Argument::nargs(int n) {
    if (_type != ARGUMENT) {
        throw Exception("called nargs() on an option");
    }
    return min_args(n).max_args(n);
}

Argument& Argument::min_args(int n) {
    if (_type != ARGUMENT) {
        throw Exception("called min_args() on an option");
    }
    _min_args = n;
    return *this;
}

Argument& Argument::max_args(int n) {
    if (_type != ARGUMENT) {
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

template <typename T>
bool store_float_argument(T& to, StringSlice value, PrintTarget error) {
    if (string_to_float(value, to)) {
        return true;
    } else {
        print(error, "invalid number");
        return false;
    }
}

bool store_argument(int8_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(uint8_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(int16_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(uint16_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(int32_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(uint32_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(int64_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(uint64_t& to, StringSlice value, PrintTarget error) { return store_integral_argument(to, value, error); }
bool store_argument(float& to, StringSlice value, PrintTarget error) { return store_float_argument(to, value, error); }
bool store_argument(double& to, StringSlice value, PrintTarget error) { return store_float_argument(to, value, error); }

void print_to(PrintTarget out, ParserUsage usage) {
    usage.parser.print_usage_to(out);
}

void print_to(PrintTarget out, ParserHelp help) {
    help.parser.print_help_to(out);
}

struct HelpAction : public Action::Impl {
    HelpAction(const Parser& parser, int exit_code): parser(parser), exit_code(exit_code) { }
    virtual bool takes_value() const { return false; }
    virtual bool process(PrintTarget error) const {
        print(io::err, parser.help());
        exit(exit_code);
    }
    const Parser& parser;
    int exit_code;
};

Action help(const Parser& parser, int exit_code) {
    return shared_ptr<Action::Impl>(new HelpAction(parser, exit_code));
}

struct VersionAction : public Action::Impl {
    VersionAction(StringSlice string): string(string) { }
    virtual bool takes_value() const { return false; }
    virtual bool process(PrintTarget error) const {
        print(io::err, string);
        exit(0);
    }
    StringSlice string;
};

Action version(StringSlice string) {
    return shared_ptr<Action::Impl>(new VersionAction(string));
}

}  // namespace args
}  // namespace sfz
