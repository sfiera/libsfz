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
    State(const Parser& spec, StringSlice program_name):
            _spec(spec),
            _program_name(program_name),
            _expecting_value(false),
            _argument_index(0) { }

    void parse_args(const vector<StringSlice>& args) {
        bool saw_dash_dash = false;
        SFZ_FOREACH(size_t i, range<size_t>(1, args.size()), {
            StringSlice token = args.at(i);
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

    bool long_option_takes_value(StringSlice option) {
        return _spec._long_options_by_name.find(option)->second->_action.takes_value();
    }

    void process_long_option(StringSlice option) {
        _spec._long_options_by_name.find(option)->second->_action.process();
    }

    void process_long_option(StringSlice option, StringSlice value) {
        _spec._long_options_by_name.find(option)->second->_action.process(value);
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

    bool short_option_takes_value(Rune option) {
        return _spec._short_options_by_name.find(option)->second->_action.takes_value();
    }

    void process_short_option(Rune option) {
        _spec._short_options_by_name.find(option)->second->_action.process();
    }

    void process_short_option(Rune option, StringSlice value) {
        _spec._short_options_by_name.find(option)->second->_action.process(value);
    }

    void process_argument(StringSlice value) {
        if (_argument_index >= _spec._argument_specs.size()) {
            throw Exception("too many arguments");
        }
        _spec._argument_specs[_argument_index]->_action.process(value);
        ++_argument_index;
    }

  private:
    const Parser& _spec;
    const StringSlice _program_name;

    Rune _option_expected;
    bool _expecting_value;

    size_t _argument_index;

    DISALLOW_COPY_AND_ASSIGN(State);
};

Parser::Parser(PrintItem description):
        _description(description) { }

Argument& Parser::add_argument(PrintItem name, Action action) {
    String printed_name(name);
    if (printed_name.empty()) {
        throw Exception("invalid argument name");
    }
    if (printed_name.at(0) == '-') {
        if (is_valid_short_option(printed_name)) {
            linked_ptr<Argument> arg(new Argument(action));
            _short_options_by_name[printed_name.at(1)] = arg;
            return *arg;
        } else if (is_valid_long_option(printed_name)) {
            linked_ptr<Argument> arg(new Argument(action));
            _long_options_by_name[printed_name] = arg;
            return *arg;
        } else {
            throw Exception("invalid argument name");
        }
    } else {
        linked_ptr<Argument> arg(new Argument(action));
        _argument_specs.push_back(arg);
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
    linked_ptr<Argument> arg(new Argument(action));
    _short_options_by_name[printed_short_name.at(1)] = arg;
    _long_options_by_name[printed_long_name] = arg;
    return *arg;
}

void Parser::parse_args(const vector<StringSlice>& args) const {
    if (args.size() < 1) {
        throw Exception("must have at least 1 arg");
    }
    State(*this, args[0]).parse_args(args);
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

void Parser::usage(StringSlice program_name) const {
}

class Action::Impl {
  public:
    virtual ~Impl() { }
    virtual bool takes_value() const = 0;
    virtual void process() const { }
    virtual void process(StringSlice value) const { }
};

namespace {

struct StoreAction : public Action::Impl {
    StoreAction(String& arg): arg(arg) { }
    virtual bool takes_value() const { return true; }
    virtual void process(StringSlice value) const { arg.assign(value); }
    String& arg;
};

}  // namespace

Action store(String& arg) {
    return linked_ptr<Action::Impl>(new StoreAction(arg));
}

namespace {

template <typename T>
struct StoreConstAction : public Action::Impl {
    StoreConstAction(T& arg, T constant): arg(arg), constant(constant) { }
    virtual bool takes_value() const { return false; }
    virtual void process() const { arg = constant; }
    T& arg;
    T constant;
};

template <>
struct StoreConstAction<String> : public Action::Impl {
    StoreConstAction(String& arg, PrintItem constant): arg(arg), constant(constant) { }
    virtual bool takes_value() const { return false; }
    virtual void process() const { arg.assign(constant); }
    String& arg;
    String constant;
};

}  // namespace

Action store_const(bool& arg, bool constant) {
    return linked_ptr<Action::Impl>(new StoreConstAction<bool>(arg, constant));
}

Action store_const(int& arg, int constant) {
    return linked_ptr<Action::Impl>(new StoreConstAction<int>(arg, constant));
}

Action store_const(String& arg, PrintItem constant) {
    return linked_ptr<Action::Impl>(new StoreConstAction<String>(arg, constant));
}

namespace {

struct IncrementAction : public Action::Impl {
    IncrementAction(int& arg): arg(arg) { }
    virtual bool takes_value() const { return false; }
    virtual void process() const { arg += 1; }
    int& arg;
};

}  // namespace

Action increment(int& arg) {
    return linked_ptr<Action::Impl>(new IncrementAction(arg));
}

Action::Action(const linked_ptr<Impl>& impl): _impl(impl) { }
Action::Action(const Action& other): _impl(other._impl) { }
Action& Action::operator=(const Action& other) { _impl = other._impl; return *this; }
Action::~Action() { }
bool Action::takes_value() const { return _impl->takes_value(); }
void Action::process() const { return _impl->process(); }
void Action::process(StringSlice value) const { return _impl->process(value); }

Argument::Argument(Action action):
        _action(action) { }

Argument& Argument::help(PrintItem s) {
    _help.assign(s);
    return *this;
}

}  // namespace args
}  // namespace sfz
