// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#ifndef RGOS_JSON_HPP_
#define RGOS_JSON_HPP_

#include <map>
#include <memory>
#include <sfz/string-map.hpp>
#include <vector>

namespace sfz {

struct JsonPrettyPrinter;
class JsonVisitor;

class Json {
  public:
    static Json object(const StringMap<Json>& value);
    static Json array(const std::vector<Json>& value);
    static Json string(const PrintItem& value);
    static Json number(double value);
    static Json boolean(bool value);

    Json();
    Json(const Json& other);
    Json& operator=(const Json& other);
    ~Json();

    void accept(const JsonVisitor& visitor) const;

    bool is_object() const;
    bool is_array() const;
    bool is_string() const;
    bool is_number() const;
    bool is_boolean() const;
    bool is_null() const;

    bool has(StringSlice key) const;
    Json get(StringSlice key) const;

    Json at(size_t index) const;
    size_t size() const;

    const StringSlice string() const;
    const double      number() const;
    const bool        boolean() const;

  private:
    friend bool operator==(const Json& x, const Json& y);

    class Value;
    class Object;
    class Array;
    class String;
    class Number;
    class Boolean;
    class Null;

    Json(Value* value);

    std::shared_ptr<const Value> _value;

    // ALLOW_COPY_AND_ASSIGN
};

bool operator==(const Json& x, const Json& y);
bool operator!=(const Json& x, const Json& y);

class JsonVisitor {
  public:
    virtual ~JsonVisitor();

    virtual void visit_object(const StringMap<Json>& value) const  = 0;
    virtual void visit_array(const std::vector<Json>& value) const = 0;
    virtual void visit_string(const StringSlice& value) const      = 0;
    virtual void visit_number(double value) const                  = 0;
    virtual void visit_bool(bool value) const                      = 0;
    virtual void visit_null() const                                = 0;
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

bool string_to_json(StringSlice string, Json& out);

JsonPrettyPrinter pretty_print(const Json& value);

void print_to(PrintTarget out, const Json& json);
void print_to(PrintTarget out, const JsonPrettyPrinter& j);

struct JsonPrettyPrinter {
    const Json& json;
};

}  // namespace sfz

#endif  // RGOS_JSON_HPP_
