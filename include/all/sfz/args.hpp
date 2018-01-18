// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#ifndef SFZ_ARGS_HPP_
#define SFZ_ARGS_HPP_

#include <limits>
#include <map>
#include <pn/file>
#include <pn/string>
#include <sfz/optional.hpp>
#include <stdexcept>
#include <vector>

namespace sfz {
namespace args {

struct callbacks;

void parse(size_t argc, char* const* argv, const callbacks& callbacks);

struct callbacks {
    using get_value_f = std::function<pn::string_view()>;

    std::function<bool(pn::rune opt, const get_value_f& get_value)>        short_option;
    std::function<bool(pn::string_view opt, const get_value_f& get_value)> long_option;
    std::function<bool(pn::string_view arg)>                               argument;
};

template <typename T>
void integer_option(pn::string_view value, T* out);

template <typename T>
void float_option(pn::string_view value, T* out);

template <typename T>
void increment_option(T* value, T delta);

////////////////////////////////////////////////////////////////////////

template <typename T>
void integer_option(pn::string_view value, T* out) {
    int64_t i64;
    integer_option(value, &i64);
    if ((i64 > std::numeric_limits<T>::max()) || (i64 < std::numeric_limits<T>::min())) {
        throw std::runtime_error(
                pn::format("integer overflow", pn::dump(value, pn::dump_short)).data());
    }
    *out = i64;
}

template <typename T>
void integer_option(pn::string_view value, ::sfz::optional<T>* out) {
    T i;
    integer_option(value, &i);
    out->emplace(i);
}

template <>
void integer_option<int64_t>(pn::string_view value, int64_t* out);

template <typename T>
void float_option(pn::string_view value, T* out) {
    double d;
    float_option(value, &d);
    if ((d > std::numeric_limits<T>::max()) || (d < -std::numeric_limits<T>::max())) {
        throw std::runtime_error(
                pn::format("float overflow", pn::dump(value, pn::dump_short)).data());
    }
    *out = d;
}

template <typename T>
void float_option(pn::string_view value, ::sfz::optional<T>* out) {
    T d;
    float_option(value, &d);
    out->emplace(d);
}

template <typename T>
void float_option(pn::string_view value, T* out);

template <typename T>
void increment_option(T* value, T delta) {
    if (delta == 0) {
        return;
    } else if (delta > 0) {
        if (*value > (std::numeric_limits<T>::max() - delta)) {
            throw std::runtime_error("integer overflow");
        }
    } else {
        if (*value < (std::numeric_limits<T>::min() - delta)) {
            throw std::runtime_error("integer overflow");
        }
    }
    *value += delta;
}

}  // namespace args
}  // namespace sfz

#endif  // SFZ_ARGS_HPP_
