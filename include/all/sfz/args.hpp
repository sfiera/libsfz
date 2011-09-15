// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_ARGS_HPP_
#define SFZ_ARGS_HPP_

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

class Parser {
  public:
    Parser(PrintItem program_name, PrintItem description);
    Parser(const char* program_name, PrintItem description);

    Argument& add_argument(PrintItem name, Action action);
    Argument& add_argument(PrintItem short_name, PrintItem long_name, Action action);

    bool parse_args(const std::vector<StringSlice>& args, PrintTarget error) const;
    bool parse_args(int argc, const char* const* argv, PrintTarget error) const;

    const String& program_name() const;
    ParserUsage usage() const;

  private:
    friend class Argument;
    friend void print_to(PrintTarget out, ParserUsage usage);

    class State;

    void print_usage_to(PrintTarget out) const;

    const String _program_name;
    const String _description;

    std::vector<linked_ptr<Argument> > _argument_specs;
    std::map<Rune, linked_ptr<Argument> > _short_options_by_name;
    StringMap<linked_ptr<Argument> > _long_options_by_name;

    DISALLOW_COPY_AND_ASSIGN(Parser);
};

template <typename To>
Action store(To& to);

template <typename To, typename Constant>
Action store_const(To& to, const Constant& constant);

template <typename To>
Action increment(To& to);

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

    Action(const linked_ptr<Impl>& impl);
    Action(const Action& other);
    Action& operator=(const Action& other);
    ~Action();

    bool takes_value() const;
    bool process(PrintTarget error) const;
    bool process(StringSlice value, PrintTarget error) const;

  private:
    linked_ptr<Impl> _impl;
};

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

    Argument(bool option, Action action);

    const bool _option;
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

template <typename To>
struct StoreAction : public Action::Impl {
    StoreAction(To& to): to(to) { }
    virtual bool takes_value() const { return true; }
    virtual bool process(StringSlice value, PrintTarget error) const { return store_argument(to, value, error); }
    To& to;
};

template <typename To>
Action store(To& to) {
    return linked_ptr<Action::Impl>(new StoreAction<To>(to));
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
    return linked_ptr<Action::Impl>(new StoreConstAction<To>(to, constant));
}

template <typename ToElement>
struct AppendAction : public Action::Impl {
    AppendAction(std::vector<ToElement>& to): to(to) { }
    virtual bool takes_value() const { return true; }
    virtual bool process(StringSlice value, PrintTarget error) const {
        to.push_back(ToElement());
        store_argument(to.back(), value, error);
        return true;
    }
    std::vector<ToElement>& to;
};

template <typename ToElement>
Action append(std::vector<ToElement>& to) {
    return linked_ptr<Action::Impl>(new AppendAction<ToElement>(to));
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
    return linked_ptr<Action::Impl>(new IncrementAction<To>(to));
}

struct ParserUsage {
    const Parser& parser;
};
void print_to(PrintTarget out, ParserUsage usage);

}  // namespace args
}  // namespace sfz

#endif  // SFZ_ARGS_HPP_
