// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/json.hpp>

#include <math.h>
#include <sfz/foreach.hpp>
#include <sfz/format.hpp>
#include <sfz/ref-count.hpp>
#include <sfz/string.hpp>
#include <sfz/string-map.hpp>

using std::map;
using std::vector;

namespace sfz {

class Json::Value : public ReferenceCounted {
  public:
    virtual void accept(JsonVisitor* visitor) const = 0;
};

class Json::Object : public Json::Value {
  public:
    Object(const StringMap<Json>& value)
        : _value(value) { }

    virtual void accept(JsonVisitor* visitor) const {
        visitor->visit_object(_value);
    }

  private:
    const StringMap<Json> _value;

    DISALLOW_COPY_AND_ASSIGN(Object);
};

class Json::Array : public Json::Value {
  public:
    Array(const vector<Json>& value)
        : _value(value) { }

    virtual void accept(JsonVisitor* visitor) const {
        visitor->visit_array(_value);
    }

  private:
    const vector<Json> _value;

    DISALLOW_COPY_AND_ASSIGN(Array);
};

class Json::String : public Json::Value {
  public:
    explicit String(const sfz::PrintItem& s)
        : _value(s) { }

    virtual void accept(JsonVisitor* visitor) const {
        visitor->visit_string(_value);
    }

  private:
    const sfz::String _value;

    DISALLOW_COPY_AND_ASSIGN(String);
};

class Json::Number : public Json::Value {
  public:
    explicit Number(double value)
        : _value(value) { }

    virtual void accept(JsonVisitor* visitor) const {
        visitor->visit_number(_value);
    }

  private:
    const double _value;

    DISALLOW_COPY_AND_ASSIGN(Number);
};

class Json::Bool : public Json::Value {
  public:
    explicit Bool(bool value)
        : _value(value) { }

    virtual void accept(JsonVisitor* visitor) const {
        visitor->visit_bool(_value);
    }

  private:
    const bool _value;

    DISALLOW_COPY_AND_ASSIGN(Bool);
};

Json Json::object(const StringMap<Json>& value) {
    return Json(new Object(value));
}

Json Json::array(const vector<Json>& value) {
    return Json(new Array(value));
}

Json Json::string(const sfz::PrintItem& value) {
    return Json(new String(value));
}

Json Json::number(double value) {
    return Json(new Number(value));
}

Json Json::bool_(bool value) {
    return Json(new Bool(value));
}

Json::Json() { }

Json::Json(Json::Value* value)
    : _value(value) { }

Json::Json(const Json& other)
    : _value(other._value) { }

Json& Json::operator=(const Json& other) {
    _value = other._value;
    return *this;
}

Json::~Json() { }

void Json::accept(JsonVisitor* visitor) const {
    if (_value.get()) {
        _value->accept(visitor);
    } else {
        visitor->visit_null();
    }
}

JsonVisitor::~JsonVisitor() { }

void JsonDefaultVisitor::visit_object(const StringMap<Json>& value) {
    visit_default("object");
}

void JsonDefaultVisitor::visit_array(const vector<Json>& value) {
    visit_default("array");
}

void JsonDefaultVisitor::visit_string(const StringSlice& value) {
    visit_default("string");
}

void JsonDefaultVisitor::visit_number(double value) {
    visit_default("number");
}

void JsonDefaultVisitor::visit_bool(bool value) {
    visit_default("bool");
}

void JsonDefaultVisitor::visit_null() {
    visit_default("null");
}

namespace {

class SerializerVisitor : public JsonVisitor {
  public:
    explicit SerializerVisitor(PrintTarget out);

    virtual void visit_object(const StringMap<Json>& value);
    virtual void visit_array(const vector<Json>& value);
    virtual void visit_string(const StringSlice& value);
    virtual void visit_number(double value);
    virtual void visit_bool(bool value);
    virtual void visit_null();

  protected:
    PrintTarget _out;

  private:
    DISALLOW_COPY_AND_ASSIGN(SerializerVisitor);
};

class PrettyPrinterVisitor : public SerializerVisitor {
  public:
    explicit PrettyPrinterVisitor(PrintTarget out);

    virtual void visit_object(const StringMap<Json>& value);
    virtual void visit_array(const vector<Json>& value);

  private:
    int _depth;

    DISALLOW_COPY_AND_ASSIGN(PrettyPrinterVisitor);
};

SerializerVisitor::SerializerVisitor(PrintTarget out)
    : _out(out) { }

void SerializerVisitor::visit_object(const StringMap<Json>& value) {
    _out.push(1, '{');
    if (value.size() > 0) {
        bool first = true;
        SFZ_FOREACH(const StringMap<Json>::value_type& item, value, {
            if (first) {
                first = false;
            } else {
                _out.push(1, ',');
            }
            print_to(_out, quote(item.first));
            _out.push(1, ':');
            item.second.accept(this);
        });
    }
    _out.push(1, '}');
}

void SerializerVisitor::visit_array(const vector<Json>& value) {
    _out.push(1, '[');
    if (value.size() > 0) {
        bool first = true;
        SFZ_FOREACH(const Json& item, value, {
            if (first) {
                first = false;
            } else {
                _out.push(1, ',');
            }
            item.accept(this);
        });
    }
    _out.push(1, ']');
}

void SerializerVisitor::visit_string(const StringSlice& value) {
    print_to(_out, quote(value));
}

void SerializerVisitor::visit_number(double value) {
    PrintItem(value).print_to(_out);
}

void SerializerVisitor::visit_bool(bool value) {
    PrintItem(value).print_to(_out);
}

void SerializerVisitor::visit_null() {
    _out.push("null");
}

PrettyPrinterVisitor::PrettyPrinterVisitor(PrintTarget out)
    : SerializerVisitor(out),
      _depth(0) { }

void PrettyPrinterVisitor::visit_object(const StringMap<Json>& value) {
    _out.push(1, '{');
    if (value.size() > 0) {
        _depth += 2;
        bool first = true;
        SFZ_FOREACH(const StringMap<Json>::value_type& item, value, {
            if (first) {
                first = false;
            } else {
                _out.push(1, ',');
            }
            _out.push(1, '\n');
            _out.push(_depth, ' ');
            print_to(_out, quote(item.first));
            _out.push(": ");
            item.second.accept(this);
        });
        _depth -= 2;
        _out.push(1, '\n');
        _out.push(_depth, ' ');
    }
    _out.push(1, '}');
}

void PrettyPrinterVisitor::visit_array(const vector<Json>& value) {
    _out.push(1, '[');
    if (value.size() > 0) {
        _depth += 2;
        bool first = true;
        SFZ_FOREACH(const Json& item, value, {
            if (first) {
                first = false;
            } else {
                _out.push(1, ',');
            }
            _out.push(1, '\n');
            _out.push(_depth, ' ');
            item.accept(this);
        });
        _depth -= 2;
        _out.push(1, '\n');
        _out.push(_depth, ' ');
    }
    _out.push(1, ']');
}

}  // namespace

JsonPrettyPrinter pretty_print(const Json& value) {
    JsonPrettyPrinter result = { value };
    return result;
}

void print_to(sfz::PrintTarget out, const Json& json) {
    SerializerVisitor visitor(out);
    json.accept(&visitor);
}

void print_to(sfz::PrintTarget out, const JsonPrettyPrinter& json) {
    PrettyPrinterVisitor visitor(out);
    json.json.accept(&visitor);
}

}  // namespace sfz
