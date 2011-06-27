// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef RGOS_JSON_HPP_
#define RGOS_JSON_HPP_

#include <map>
#include <vector>
#include <sfz/memory.hpp>
#include <sfz/string-map.hpp>

namespace sfz {

struct JsonPrettyPrinter;
class JsonVisitor;

class Json {
  public:
    static Json object(const StringMap<Json>& value);
    static Json array(const std::vector<Json>& value);
    static Json string(const PrintItem& value);
    static Json number(double value);
    static Json bool_(bool value);

    Json();
    Json(const Json& other);
    Json& operator=(const Json& other);
    ~Json();

    void accept(const JsonVisitor& visitor) const;

  private:
    class Value;
    class Object;
    class Array;
    class String;
    class Number;
    class Bool;
    class Null;

    Json(Value* value);

    scoped_ref<const Value> _value;

    // ALLOW_COPY_AND_ASSIGN
};

class JsonVisitor {
  public:
    virtual ~JsonVisitor();

    virtual void visit_object(const StringMap<Json>& value) const = 0;
    virtual void visit_array(const std::vector<Json>& value) const = 0;
    virtual void visit_string(const StringSlice& value) const = 0;
    virtual void visit_number(double value) const = 0;
    virtual void visit_bool(bool value) const = 0;
    virtual void visit_null() const = 0;
};

class JsonDefaultVisitor : public JsonVisitor {
  public:
    virtual void visit_object(const StringMap<Json>& value) const;
    virtual void visit_array(const std::vector<Json>& value) const;
    virtual void visit_string(const StringSlice& value) const;
    virtual void visit_number(double value) const;
    virtual void visit_bool(bool value) const;
    virtual void visit_null() const;

    virtual void visit_default(const char* type) const = 0;
};

JsonPrettyPrinter pretty_print(const Json& value);

void print_to(PrintTarget out, const Json& json);
void print_to(PrintTarget out, const JsonPrettyPrinter& j);

struct JsonPrettyPrinter { const Json& json; };

}  // namespace sfz

#endif  // RGOS_JSON_HPP_
