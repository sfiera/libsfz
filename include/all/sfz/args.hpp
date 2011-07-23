// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_ARGS_HPP_
#define SFZ_ARGS_HPP_

#include <map>
#include <vector>
#include <sfz/macros.hpp>
#include <sfz/print.hpp>
#include <sfz/string.hpp>
#include <sfz/string-map.hpp>

namespace sfz {
namespace args {

class Action;
class Argument;

class Parser {
  public:
    Parser(PrintItem description);

    Argument& add_argument(PrintItem name, Action action);
    Argument& add_argument(PrintItem short_name, PrintItem long_name, Action action);

    void parse_args(const std::vector<StringSlice>& args) const;
    void parse_args(int argc, const char* const* argv) const;

    void usage(StringSlice program_name) const;

  private:
    friend class Argument;

    class State;

    const String _description;
    std::vector<linked_ptr<Argument> > _argument_specs;
    std::map<Rune, linked_ptr<Argument> > _short_options_by_name;
    StringMap<linked_ptr<Argument> > _long_options_by_name;

    DISALLOW_COPY_AND_ASSIGN(Parser);
};

Action store(String& arg);
Action store_const(bool& arg, bool constant);
Action store_const(int& arg, int constant);
Action store_const(String& arg, PrintItem constant);
Action increment(int& arg);

class Action {
  public:
    class Impl;

    Action(const linked_ptr<Impl>& impl);
    Action(const Action& other);
    Action& operator=(const Action& other);
    ~Action();

    bool takes_value() const;
    void process() const;
    void process(StringSlice value) const;

  private:
    linked_ptr<Impl> _impl;
};

class Argument {
  public:
    Argument& help(PrintItem s);

  private:
    friend class Parser;
    friend class Parser::State;

    Argument(Action action);

    Action _action;
    String _help;

    DISALLOW_COPY_AND_ASSIGN(Argument);
};

}  // namespace args
}  // namespace sfz

#endif  // SFZ_ARGS_HPP_
