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

namespace {

const char kEscaped[' '][7] = {
    "\\u0000", "\\u0001", "\\u0002", "\\u0003", "\\u0004", "\\u0005", "\\u0006", "\\u0007",
    "\\b",     "\\t",     "\\n",     "\\u0013", "\\f",     "\\r",     "\\u0016", "\\u0017",
    "\\u0020", "\\u0021", "\\u0022", "\\u0023", "\\u0024", "\\u0025", "\\u0026", "\\u0027",
    "\\u0030", "\\u0031", "\\u0032", "\\u0033", "\\u0034", "\\u0035", "\\u0036", "\\u0037",
};

void json_escape(PrintTarget out, const StringSlice& value) {
    SFZ_FOREACH(Rune rune, value, {
        if (rune < ' ') {
            out.push(kEscaped[rune]);
        } else if (rune == '\"' || rune == '\\' || rune == '/') {
            out.push(1, '\\');
            out.push(1, rune);
        } else {
            out.push(1, rune);
        }
    });
}

}  // namespace

class Json::Value : public ReferenceCounted {
  public:
    virtual void accept(const JsonVisitor& visitor) const = 0;
};

class Json::Object : public Json::Value {
  public:
    Object(const StringMap<Json>& value)
        : _value(value) { }

    virtual void accept(const JsonVisitor& visitor) const {
        visitor.visit_object(_value);
    }

  private:
    const StringMap<Json> _value;

    DISALLOW_COPY_AND_ASSIGN(Object);
};

class Json::Array : public Json::Value {
  public:
    Array(const vector<Json>& value)
        : _value(value) { }

    virtual void accept(const JsonVisitor& visitor) const {
        visitor.visit_array(_value);
    }

  private:
    const vector<Json> _value;

    DISALLOW_COPY_AND_ASSIGN(Array);
};

class Json::String : public Json::Value {
  public:
    explicit String(const sfz::PrintItem& s)
        : _value(s) { }

    virtual void accept(const JsonVisitor& visitor) const {
        visitor.visit_string(_value);
    }

  private:
    const sfz::String _value;

    DISALLOW_COPY_AND_ASSIGN(String);
};

class Json::Number : public Json::Value {
  public:
    explicit Number(double value)
        : _value(value) { }

    virtual void accept(const JsonVisitor& visitor) const {
        visitor.visit_number(_value);
    }

  private:
    const double _value;

    DISALLOW_COPY_AND_ASSIGN(Number);
};

class Json::Bool : public Json::Value {
  public:
    explicit Bool(bool value)
        : _value(value) { }

    virtual void accept(const JsonVisitor& visitor) const {
        visitor.visit_bool(_value);
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

void Json::accept(const JsonVisitor& visitor) const {
    if (_value.get()) {
        _value->accept(visitor);
    } else {
        visitor.visit_null();
    }
}

JsonVisitor::~JsonVisitor() { }

void JsonDefaultVisitor::visit_object(const StringMap<Json>& value) const {
    visit_default("object");
}

void JsonDefaultVisitor::visit_array(const vector<Json>& value) const {
    visit_default("array");
}

void JsonDefaultVisitor::visit_string(const StringSlice& value) const {
    visit_default("string");
}

void JsonDefaultVisitor::visit_number(double value) const {
    visit_default("number");
}

void JsonDefaultVisitor::visit_bool(bool value) const {
    visit_default("bool");
}

void JsonDefaultVisitor::visit_null() const {
    visit_default("null");
}

namespace {

class SerializerVisitor : public JsonVisitor {
  public:
    explicit SerializerVisitor(PrintTarget out);

    virtual void visit_object(const StringMap<Json>& value) const;
    virtual void visit_array(const vector<Json>& value) const;
    virtual void visit_string(const StringSlice& value) const;
    virtual void visit_number(double value) const;
    virtual void visit_bool(bool value) const;
    virtual void visit_null() const;

  protected:
    PrintTarget& _out;
};

class PrettyPrinterVisitor : public SerializerVisitor {
  public:
    explicit PrettyPrinterVisitor(int& depth, PrintTarget out);

    virtual void visit_object(const StringMap<Json>& value) const;
    virtual void visit_array(const vector<Json>& value) const;

  private:
    int& _depth;
};

SerializerVisitor::SerializerVisitor(PrintTarget out)
    : _out(out) { }

void SerializerVisitor::visit_object(const StringMap<Json>& value) const {
    _out.push(1, '{');
    if (value.size() > 0) {
        bool first = true;
        SFZ_FOREACH(const StringMap<Json>::value_type& item, value, {
            if (first) {
                first = false;
            } else {
                _out.push(1, ',');
            }
            _out.push(1, '"');
            json_escape(_out, item.first);
            _out.push(1, '"');
            _out.push(1, ':');
            item.second.accept(*this);
        });
    }
    _out.push(1, '}');
}

void SerializerVisitor::visit_array(const vector<Json>& value) const {
    _out.push(1, '[');
    if (value.size() > 0) {
        bool first = true;
        SFZ_FOREACH(const Json& item, value, {
            if (first) {
                first = false;
            } else {
                _out.push(1, ',');
            }
            item.accept(*this);
        });
    }
    _out.push(1, ']');
}

void SerializerVisitor::visit_string(const StringSlice& value) const {
    _out.push(1, '"');
    json_escape(_out, value);
    _out.push(1, '"');
}

void SerializerVisitor::visit_number(double value) const {
    PrintItem(value).print_to(_out);
}

void SerializerVisitor::visit_bool(bool value) const {
    PrintItem(value).print_to(_out);
}

void SerializerVisitor::visit_null() const {
    _out.push("null");
}

PrettyPrinterVisitor::PrettyPrinterVisitor(int& depth, PrintTarget out)
    : SerializerVisitor(out),
      _depth(depth) { }

void PrettyPrinterVisitor::visit_object(const StringMap<Json>& value) const {
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
            _out.push(1, '"');
            json_escape(_out, item.first);
            _out.push("\": ");
            item.second.accept(*this);
        });
        _depth -= 2;
        _out.push(1, '\n');
        _out.push(_depth, ' ');
    }
    _out.push(1, '}');
}

void PrettyPrinterVisitor::visit_array(const vector<Json>& value) const {
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
            item.accept(*this);
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
    json.accept(SerializerVisitor(out));
}

void print_to(sfz::PrintTarget out, const JsonPrettyPrinter& json) {
    int depth = 0;
    json.json.accept(PrettyPrinterVisitor(depth, out));
}

}  // namespace sfz
