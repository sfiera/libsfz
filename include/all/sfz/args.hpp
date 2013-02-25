// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_ARGS_HPP_
#define SFZ_ARGS_HPP_

#include <limits>
#include <map>
#include <vector>
#include <sfz/algorithm.hpp>
#include <sfz/exception.hpp>
#include <sfz/macros.hpp>
#include <sfz/print.hpp>
#include <sfz/string.hpp>
#include <sfz/string-map.hpp>

namespace sfz {
namespace args {

class Action;
class Argument;
struct ParserUsage;
struct ParserHelp;
Action noop();

class Action {
  public:
    class Impl {
      public:
        virtual ~Impl() { }
        virtual bool takes_value() const = 0;
        virtual bool process(PrintTarget error) const {
            print(error, "not implemented");
            return false;
        }
        virtual bool process(StringSlice value, PrintTarget error) const {
            print(error, "not implemented");
            return false;
        }
    };

    Action(const std::shared_ptr<Impl>& impl);
    Action(const Action& other);
    Action& operator=(const Action& other);
    ~Action();

    bool takes_value() const;
    bool process(PrintTarget error) const;
    bool process(StringSlice value, PrintTarget error) const;

  private:
    std::shared_ptr<Impl> _impl;
};

class Parser {
  public:
    Parser(PrintItem program_name, PrintItem description, Action = noop());
    Parser(const char* program_name, PrintItem description, Action = noop());

    Argument& add_argument(PrintItem name, Action action);
    Argument& add_argument(PrintItem short_name, PrintItem long_name, Action action);
    Parser& add_subparser(PrintItem name, PrintItem description, Action action);

    bool parse_args(const std::vector<StringSlice>& args, PrintTarget error) const;
    bool parse_args(int argc, const char* const* argv, PrintTarget error) const;

    const String& name() const;
    ParserUsage usage() const;
    ParserHelp help() const;

  private:
    friend class Argument;
    friend void print_to(PrintTarget out, ParserUsage usage);
    friend void print_to(PrintTarget out, ParserHelp help);

    class State;

    Parser(Parser* const parent, PrintItem name, PrintItem description, Action action);

    const Argument& long_option(StringSlice option) const;
    bool has_long_option(StringSlice option) const;
    bool long_option_takes_value(StringSlice option) const;

    const Argument& short_option(Rune option) const;
    bool has_short_option(Rune option) const;
    bool short_option_takes_value(Rune option) const;

    bool has_subparsers() const;
    bool has_subparser(StringSlice name) const;
    const Parser& subparser(StringSlice name) const;

    void print_usage_to(PrintTarget out) const;
    void print_help_to(PrintTarget out) const;

    const Parser* const _parent;
    const String _name;
    const String _description;
    const Action _action;

    std::vector<std::shared_ptr<Parser> > _subparsers;
    StringMap<std::shared_ptr<Parser> > _subparsers_by_name;

    std::vector<std::shared_ptr<Argument> > _argument_specs;
    std::shared_ptr<Argument> _subparser_argument;

    std::vector<std::shared_ptr<Argument> > _option_specs;
    std::map<Rune, std::shared_ptr<Argument> > _short_options_by_name;
    StringMap<std::shared_ptr<Argument> > _long_options_by_name;

    DISALLOW_COPY_AND_ASSIGN(Parser);
};

template <typename To>
Action store(To& to);

template <typename To, typename Constant>
Action store_const(To& to, const Constant& constant);

template <typename To>
Action increment(To& to);

class Argument {
  public:
    Argument& help(PrintItem s);
    Argument& metavar(PrintItem s);
    Argument& required();
    Argument& nargs(int n);
    Argument& min_args(int n);
    Argument& max_args(int n);

  private:
    friend class Parser;
    friend class Parser::State;

    enum Type {
        ARGUMENT,
        SHORT_OPTION,
        LONG_OPTION,
        BOTH_OPTION,
    };

    Argument(Type type, const StringSlice& short_option_name,
            const StringSlice& long_option_name, Action action);

    const Type _type;
    const String _short_option_name;
    const String _long_option_name;
    const Action _action;
    String _metavar;
    String _help;
    int _min_args;
    int _max_args;

    DISALLOW_COPY_AND_ASSIGN(Argument);
};

bool store_argument(bool& to, StringSlice value, PrintTarget error);

bool store_argument(int8_t& to, StringSlice value, PrintTarget error);
bool store_argument(uint8_t& to, StringSlice value, PrintTarget error);
bool store_argument(int16_t& to, StringSlice value, PrintTarget error);
bool store_argument(uint16_t& to, StringSlice value, PrintTarget error);
bool store_argument(int32_t& to, StringSlice value, PrintTarget error);
bool store_argument(uint32_t& to, StringSlice value, PrintTarget error);
bool store_argument(int64_t& to, StringSlice value, PrintTarget error);
bool store_argument(uint64_t& to, StringSlice value, PrintTarget error);
bool store_argument(float& to, StringSlice value, PrintTarget error);
bool store_argument(double& to, StringSlice value, PrintTarget error);

template <typename To>
struct StoreAction : public Action::Impl {
    StoreAction(To& to): to(to) { }
    virtual bool takes_value() const { return true; }
    virtual bool process(StringSlice value, PrintTarget error) const { return store_argument(to, value, error); }
    To& to;
};

template <typename To>
Action store(To& to) {
    return std::shared_ptr<Action::Impl>(new StoreAction<To>(to));
}

template <typename To>
struct StoreConstAction : public Action::Impl {
    template <typename Constant>
    StoreConstAction(To& to, Constant constant): to(to), constant(constant) { }
    virtual bool takes_value() const { return false; }
    virtual bool process(PrintTarget error) const { copy(to, constant); return true; }
    To& to;
    To constant;
};

template <typename To, typename Constant>
Action store_const(To& to, const Constant& constant) {
    return std::shared_ptr<Action::Impl>(new StoreConstAction<To>(to, constant));
}

template <typename ToElement>
struct AppendAction : public Action::Impl {
    AppendAction(std::vector<ToElement>& to): to(to) { }
    virtual bool takes_value() const { return true; }
    virtual bool process(StringSlice value, PrintTarget error) const {
        ToElement next;
        if (!store_argument(next, value, error)) {
            return false;
        }
        to.push_back(next);
        return true;
    }
    std::vector<ToElement>& to;
};

template <typename ToElement>
Action append(std::vector<ToElement>& to) {
    return std::shared_ptr<Action::Impl>(new AppendAction<ToElement>(to));
}

template <typename To>
struct IncrementAction : public Action::Impl {
    IncrementAction(To& arg): arg(arg) { }
    virtual bool takes_value() const { return false; }
    virtual bool process(PrintTarget error) const {
        if (arg == std::numeric_limits<To>::max()) {
            print(error, "integer overflow");
            return false;
        }
        ++arg;
        return true;
    }
    To& arg;
};

template <typename To>
Action increment(To& to) {
    return std::shared_ptr<Action::Impl>(new IncrementAction<To>(to));
}

Action help(const Parser& parser, int exit_code);
Action version(StringSlice string);

inline Action noop() { return std::shared_ptr<Action::Impl>(); }

struct ParserUsage { const Parser& parser; };
struct ParserHelp { const Parser& parser; };
void print_to(PrintTarget out, ParserUsage usage);
void print_to(PrintTarget out, ParserHelp help);

}  // namespace args
}  // namespace sfz

#endif  // SFZ_ARGS_HPP_
