// Copyright (c) 2009 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute it and/or modify it
// under the terms of the MIT License.

#include <sfz/json.hpp>

#include <math.h>
#include <sfz/encoding.hpp>
#include <sfz/format.hpp>
#include <sfz/string.hpp>
#include <sfz/string-map.hpp>
#include <sfz/string-utils.hpp>

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
    for (Rune rune: value) {
        if (rune < ' ') {
            out.push(kEscaped[rune]);
        } else if (rune == '\"' || rune == '\\' || rune == '/') {
            out.push(1, '\\');
            out.push(1, rune);
        } else {
            out.push(1, rune);
        }
    }
}

}  // namespace

class Json::Value {
  public:
    virtual void accept(const JsonVisitor& visitor) const = 0;

    virtual bool is_object() const { return false; }
    virtual bool is_array() const { return false; }
    virtual bool is_string() const { return false; }
    virtual bool is_number() const { return false; }
    virtual bool is_boolean() const { return false; }
    virtual bool is_null() const { return false; }

    virtual bool has(StringSlice key) const { return false; }
    virtual Json get(StringSlice key) const { return Json(); }

    virtual Json at(size_t index) const { return Json(); }
    virtual size_t size() const { return 0; }

    virtual const StringSlice string() const { return StringSlice(); }
    virtual const double number() const { return 0.0; }
    virtual const bool boolean() const { return false; }
};

class Json::Object : public Json::Value {
  public:
    Object(const StringMap<Json>& value)
        : _value(value) { }

    virtual void accept(const JsonVisitor& visitor) const { visitor.visit_object(_value); }
    virtual bool is_object() const { return true; }
    virtual bool has(StringSlice key) const { return _value.find(key) != _value.end(); }
    virtual Json get(StringSlice key) const {
        auto it = _value.find(key);
        if (it == _value.end()) {
            return Json();
        } else {
            return it->second;
        }
    }

  private:
    const StringMap<Json> _value;

    DISALLOW_COPY_AND_ASSIGN(Object);
};

class Json::Array : public Json::Value {
  public:
    Array(const vector<Json>& value)
        : _value(value) { }

    virtual void accept(const JsonVisitor& visitor) const { visitor.visit_array(_value); }
    virtual bool is_array() const { return true; }
    virtual size_t size() const { return _value.size(); }
    virtual Json at(size_t index) const {
        if (index < size()) {
            return _value.at(index);
        } else {
            return Json();
        }
    }

  private:
    const vector<Json> _value;

    DISALLOW_COPY_AND_ASSIGN(Array);
};

class Json::String : public Json::Value {
  public:
    explicit String(const sfz::PrintItem& s)
        : _value(s) { }

    virtual void accept(const JsonVisitor& visitor) const { visitor.visit_string(_value); }
    virtual bool is_string() const { return true; }
    virtual const StringSlice string() const { return _value; }

  private:
    const sfz::String _value;

    DISALLOW_COPY_AND_ASSIGN(String);
};

class Json::Number : public Json::Value {
  public:
    explicit Number(double value)
        : _value(value) { }

    virtual void accept(const JsonVisitor& visitor) const { visitor.visit_number(_value); }

    virtual bool is_number() const { return true; }
    virtual const double number() const { return _value; }

  private:
    const double _value;

    DISALLOW_COPY_AND_ASSIGN(Number);
};

class Json::Boolean : public Json::Value {
  public:
    explicit Boolean(bool value)
        : _value(value) { }

    virtual void accept(const JsonVisitor& visitor) const { visitor.visit_bool(_value); }
    virtual bool is_boolean() const { return true; }
    virtual const bool boolean() const { return _value; }

  private:
    const bool _value;

    DISALLOW_COPY_AND_ASSIGN(Boolean);
};

class Json::Null : public Json::Value {
  public:
    static const std::shared_ptr<Null>& shared_null() {
        static std::shared_ptr<Null> null(new Null);
        return null;
    }

    virtual void accept(const JsonVisitor& visitor) const { visitor.visit_null(); }
    virtual bool is_null() const { return true; }

  private:
    Null() { }
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

Json Json::boolean(bool value) {
    return Json(new Boolean(value));
}

Json::Json(): _value(Null::shared_null()) { }

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
    _value->accept(visitor);
}

bool Json::is_object() const { return _value->is_object(); }
bool Json::is_array() const { return _value->is_array(); }
bool Json::is_string() const { return _value->is_string(); }
bool Json::is_number() const { return _value->is_number(); }
bool Json::is_boolean() const { return _value->is_boolean(); }
bool Json::is_null() const { return _value->is_null(); }

bool Json::has(StringSlice key) const { return _value->has(key); }
Json Json::get(StringSlice key) const { return _value->get(key); }
Json Json::at(size_t index) const { return _value->at(index); }
size_t Json::size() const { return _value->size(); }

const StringSlice Json::string() const { return _value->string(); }
const double Json::number() const { return _value->number(); }
const bool Json::boolean() const { return _value->boolean(); }

namespace {

struct EqualsVisitorBase : public JsonVisitor {
    EqualsVisitorBase(bool& equal):
            equal(equal) { }

    virtual void visit_object(const StringMap<Json>& value) const { equal = false; }
    virtual void visit_array(const std::vector<Json>& value) const { equal = false; }
    virtual void visit_string(const StringSlice& value) const { equal = false; }
    virtual void visit_number(double value) const { equal = false; }
    virtual void visit_bool(bool value) const { equal = false; }
    virtual void visit_null() const { equal = false; }

    bool& equal;
};

template <typename X, typename Y>
struct EqualTypeAndEqual {
    static bool call(const X& x, const Y& y) {
        return false;
    }
};

template <typename T>
struct EqualTypeAndEqual<T, T> {
    static bool call(const T& x, const T& y) {
        return x == y;
    }
};

template <typename X, typename Y>
bool equal_type_and_equal(const X& x, const Y& y) {
    return EqualTypeAndEqual<X, Y>::call(x, y);
}

template <typename T>
struct EqualsVisitor : public EqualsVisitorBase {
    EqualsVisitor(bool& equal, const T& other):
            EqualsVisitorBase(equal),
            other(other) { }

    virtual void visit_object(const StringMap<Json>& value) const {
        equal = equal_type_and_equal(value, other);
    }

    virtual void visit_array(const vector<Json>& value) const {
        equal = equal_type_and_equal(value, other);
    }

    virtual void visit_string(const StringSlice& value) const {
        equal = equal_type_and_equal(value, other);
    }

    virtual void visit_number(double value) const {
        equal = equal_type_and_equal(value, other);
    }

    virtual void visit_bool(bool value) const {
        equal = equal_type_and_equal(value, other);
    }

    const T& other;
};

template <>
struct EqualsVisitor<void> : public EqualsVisitorBase {
    EqualsVisitor(bool& equal): EqualsVisitorBase(equal) { }
    virtual void visit_null() const { equal = true; }
};

template <>
struct EqualsVisitor<Json> : public EqualsVisitorBase {
    EqualsVisitor(bool& equal, const Json& other):
            EqualsVisitorBase(equal),
            other(other) { }

    virtual void visit_object(const StringMap<Json>& value) const {
        other.accept(EqualsVisitor<StringMap<Json> >(equal, value));
    }

    virtual void visit_array(const std::vector<Json>& value) const {
        other.accept(EqualsVisitor<std::vector<Json> >(equal, value));
    }

    virtual void visit_string(const StringSlice& value) const {
        other.accept(EqualsVisitor<StringSlice>(equal, value));
    }

    virtual void visit_number(double value) const {
        other.accept(EqualsVisitor<double>(equal, value));
    }

    virtual void visit_bool(bool value) const {
        other.accept(EqualsVisitor<bool>(equal, value));
    }

    virtual void visit_null() const {
        other.accept(EqualsVisitor<void>(equal));
    }

    const Json& other;
};

}  // namespace

bool operator==(const Json& x, const Json& y) {
    bool equal;
    x.accept(EqualsVisitor<Json>(equal, y));
    return equal;
}

bool operator!=(const Json& x, const Json& y) {
    return !(x == y);
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

bool parse_any(StringSlice& string, Json& out);
bool parse_string(StringSlice& string, String& out);

Rune shift(StringSlice& string) {
    Rune result = string.at(0);
    string = string.slice(1);
    return result;
}

StringSlice shift(StringSlice& string, size_t size) {
    StringSlice result = string.slice(0, size);
    string = string.slice(size);
    return result;
}

void consume_whitespace(StringSlice& string) {
    while (!string.empty()) {
        // TODO(sfiera): create a function for this.
        switch (string.at(0)) {
          case ' ':
          case '\t':
          case '\r':
          case '\n':
          case 0x00a0:  // NO-BREAK SPACE
          case 0x1680:  // OGHAM SPACE MARK
          case 0x2000:  // EN QUAD
          case 0x2001:  // EM QUAD
          case 0x2002:  // EN SPACE
          case 0x2003:  // EM SPACE
          case 0x2004:  // THREE-PER-EM SPACE
          case 0x2005:  // FOUR-PER-EM SPACE
          case 0x2006:  // SIX-PER-EM SPACE
          case 0x2007:  // FIGURE SPACE
          case 0x2008:  // PUNCTUATION SPACE
          case 0x2009:  // THIN SPACE
          case 0x200a:  // HAIR SPACE
          case 0x2028:  // LINE SEPARATOR
          case 0x3000:  // IDEOGRAPHIC SPACE
          case 0xfeff:  // BYTE ORDER MARK
            string = string.slice(1);
            break;
          default:
            return;
        }
    }
}

bool parse_literal(StringSlice& string, StringSlice literal) {
    StringSlice copy(string);
    consume_whitespace(copy);
    if ((copy.size() >= literal.size()) && (shift(copy, literal.size()) == literal)) {
        string = copy;
        return true;
    }
    return false;
}

bool parse_object_contents(StringSlice& string, StringMap<Json>& out) {
    while (!string.empty()) {
        String name;
        Json value;
        if (!(parse_string(string, name)
                    && parse_literal(string, ":")
                    && parse_any(string, value))) {
            return false;
        }
        out[name] = value;
        if (parse_literal(string, "}")) {
            return true;
        } else if (!parse_literal(string, ",")) {
            return false;
        }
    }
    return false;
}

bool parse_object(StringSlice& string, Json& out) {
    StringMap<Json> result;
    if (parse_literal(string, "{")
            && (parse_literal(string, "}") || parse_object_contents(string, result))) {
        out = Json::object(result);
        return true;
    }
    return false;
}

bool parse_array_contents(StringSlice& string, vector<Json>& out) {
    while (!string.empty()) {
        Json value;
        if (!parse_any(string, value)) {
            return false;
        }
        out.push_back(value);
        if (parse_literal(string, "]")) {
            return true;
        } else if (!parse_literal(string, ",")) {
            return false;
        }
    }
    return false;
}

bool parse_array(StringSlice& string, Json& out) {
    vector<Json> result;
    if (parse_literal(string, "[")
            && (parse_literal(string, "]") || parse_array_contents(string, result))) {
        out = Json::array(result);
        return true;
    }
    return false;
}

bool parse_unicode_escape(StringSlice& string, String& out) {
    uint16_t value;
    if ((string.size() >= 4) && string_to_int(shift(string, 4), value, 16)
            && is_valid_code_point(value)) {
        out.append(1, value);
        return true;
    }
    return false;
}

bool parse_escape_sequence(StringSlice& string, String& out) {
    if (string.empty()) {
        return false;
    }
    Rune next = shift(string);
    switch (next) {
      case '"':
      case '\\':
      case '/': out.append(1, next); return true;
      case 'b': out.append(1, '\b'); return true;
      case 'f': out.append(1, '\f'); return true;
      case 'n': out.append(1, '\n'); return true;
      case 'r': out.append(1, '\r'); return true;
      case 't': out.append(1, '\t'); return true;
      case 'u': return parse_unicode_escape(string, out);
    }
    return false;
}

bool parse_string_continuation(StringSlice& string, String& out) {
    while (!string.empty()) {
        Rune next = shift(string);
        switch (next) {
          default:
            out.append(1, next);
            break;
          case '\\':
            if (!parse_escape_sequence(string, out)) {
                return false;
            }
            break;
          case '"':
            return true;
        }
    }
    return false;
}

bool parse_string(StringSlice& string, String& out) {
    return parse_literal(string, "\"")
        && parse_string_continuation(string, out);
}

bool parse_string(StringSlice& string, Json& out) {
    String result;
    if (parse_string(string, result)) {
        out = Json::string(result);
        return true;
    }
    return false;
}

bool parse_number(StringSlice& string, Json& out) {
    consume_whitespace(string);
    String number_string;
    while (!string.empty()) {
        switch (string.at(0)) {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case '+':
          case '-':
          case '.':
          case 'E':
          case 'e':
            number_string.append(1, shift(string));
            continue;
        }
        break;
    }
    if (number_string.empty()) {
        return false;
    }
    double number;
    if (string_to_float(number_string, number)) {
        out = Json::number(number);
        return true;
    }
    return false;
}

bool parse_true(StringSlice& string, Json& out) {
    if (parse_literal(string, "true")) {
        out = Json::boolean(true);
        return true;
    }
    return false;
}

bool parse_false(StringSlice& string, Json& out) {
    if (parse_literal(string, "false")) {
        out = Json::boolean(false);
        return true;
    }
    return false;
}

bool parse_null(StringSlice& string, Json& out) {
    if (parse_literal(string, "null")) {
        out = Json();
        return true;
    }
    return false;
}

bool parse_any(StringSlice& string, Json& out) {
    consume_whitespace(string);
    if (string.empty()) {
        return false;
    }
    switch (string.at(0)) {
      case '{': return parse_object(string, out);
      case '[': return parse_array(string, out);
      case '"': return parse_string(string, out);
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '-': return parse_number(string, out);
      case 't': return parse_true(string, out);
      case 'f': return parse_false(string, out);
      case 'n': return parse_null(string, out);
    }
    return false;
}

}  // namespace

bool string_to_json(StringSlice string, Json& out) {
    if (!parse_any(string, out)) {
        return false;
    }
    consume_whitespace(string);
    return string.empty();
}

namespace {

class SerializerVisitor : public JsonVisitor {
  public:
    explicit SerializerVisitor(PrintTarget& out);

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
    explicit PrettyPrinterVisitor(int& depth, PrintTarget& out);

    virtual void visit_object(const StringMap<Json>& value) const;
    virtual void visit_array(const vector<Json>& value) const;

  private:
    int& _depth;
};

SerializerVisitor::SerializerVisitor(PrintTarget& out)
    : _out(out) { }

void SerializerVisitor::visit_object(const StringMap<Json>& value) const {
    _out.push(1, '{');
    if (value.size() > 0) {
        bool first = true;
        for (const StringMap<Json>::value_type& item: value) {
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
        }
    }
    _out.push(1, '}');
}

void SerializerVisitor::visit_array(const vector<Json>& value) const {
    _out.push(1, '[');
    if (value.size() > 0) {
        bool first = true;
        for (const Json& item: value) {
            if (first) {
                first = false;
            } else {
                _out.push(1, ',');
            }
            item.accept(*this);
        }
    }
    _out.push(1, ']');
}

void SerializerVisitor::visit_string(const StringSlice& value) const {
    _out.push(1, '"');
    json_escape(_out, value);
    _out.push(1, '"');
}

void SerializerVisitor::visit_number(double value) const {
    int64_t int_value = value;
    if (value == int_value) {
        PrintItem(int_value).print_to(_out);
    } else {
        PrintItem(value).print_to(_out);
    }
}

void SerializerVisitor::visit_bool(bool value) const {
    PrintItem(value).print_to(_out);
}

void SerializerVisitor::visit_null() const {
    _out.push("null");
}

PrettyPrinterVisitor::PrettyPrinterVisitor(int& depth, PrintTarget& out)
    : SerializerVisitor(out),
      _depth(depth) { }

void PrettyPrinterVisitor::visit_object(const StringMap<Json>& value) const {
    _out.push(1, '{');
    if (value.size() > 0) {
        _depth += 2;
        bool first = true;
        for (const StringMap<Json>::value_type& item: value) {
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
        }
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
        for (const Json& item: value) {
            if (first) {
                first = false;
            } else {
                _out.push(1, ',');
            }
            _out.push(1, '\n');
            _out.push(_depth, ' ');
            item.accept(*this);
        }
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
