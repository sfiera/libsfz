// Copyright (c) 2011 Chris Pickel <sfiera@gmail.com>
//
// This file is part of libsfz, a free software project.  You can redistribute
// it and/or modify it under the terms of the MIT License.

#include <sfz/args.hpp>

#include <stdlib.h>
#include <sfz/encoding.hpp>
#include <sfz/exception.hpp>
#include <sfz/format.hpp>
#include <sfz/range.hpp>
#include <sfz/string-utils.hpp>

using std::map;
using std::pair;
using std::vector;
using std::shared_ptr;

namespace sfz {
namespace args {

static bool starts_with(pn::string_view string, pn::string_view prefix) {
    return (string.size() >= prefix.size()) && (string.substr(0, prefix.size()) == prefix);
}

void parse(size_t argc, char* const* argv, const callbacks& callbacks) {
    size_t i = 0;
    for (; i < argc; ++i) {
        pn::string_view arg = argv[i];
        if (arg == "--") {  // --
            ++i;
            break;
        } else if (starts_with(arg, "--")) {
            pn::string_view            opt     = arg.substr(2);
            pn::string_view::size_type eq      = opt.find("=");
            bool                       handled = false;
            try {
                if (eq == opt.npos) {  // --option; --option value
                    bool used_value = false;
                    if (callbacks.long_option) {
                        handled = callbacks.long_option(opt, [&i, argc, argv, &used_value, opt] {
                            if (!used_value) {
                                used_value = true;
                                if (++i >= argc) {
                                    throw std::runtime_error("argument required");
                                }
                            }
                            return argv[i];
                        });
                    }
                } else {  // --option=value
                    bool            used_value = false;
                    pn::string_view value      = opt.substr(eq + 1);
                    opt                        = opt.substr(0, eq);
                    if (callbacks.long_option) {
                        handled = callbacks.long_option(opt, [&used_value, value] {
                            used_value= true;
                            return value;
                        });
                        if (handled && !used_value) {
                            throw std::runtime_error("no value permitted");
                        }
                    }
                }
            } catch (...) {
                std::throw_with_nested(std::runtime_error(arg.copy().c_str()));
            }
            if (!handled) {
                throw std::runtime_error(pn::format("illegal option: --{0}", opt).c_str());
            }
        } else if (starts_with(arg, "-") && (arg != "-")) {
            pn::string_view opts = arg.substr(1);
            while (!opts.empty()) {  // -abc; -abcvalue; -abc value
                pn::rune opt    = *opts.begin();
                opts            = opts.substr(opt.size());
                bool handled    = false;
                bool used_value = false;
                try {
                    handled = callbacks.short_option &&
                              callbacks.short_option(opt, [&i, argc, argv, &used_value, opts] {
                                  if (!opts.empty()) {
                                      used_value = true;
                                      return opts;
                                  } else if (!used_value) {
                                      if (++i >= argc) {
                                          throw std::runtime_error("argument required");
                                      }
                                      used_value = true;
                                  }
                                  return pn::string_view{argv[i]};
                              });
                } catch (...) {
                    if (used_value) {
                        if (!opts.empty()) {
                            std::throw_with_nested(
                                    std::runtime_error(pn::format("-{0}{1}", opt, opts).c_str()));
                        } else {
                            std::throw_with_nested(std::runtime_error(
                                    pn::format("-{0} {1}", opt, argv[i]).c_str()));
                        }
                    } else {
                        std::throw_with_nested(
                                std::runtime_error(pn::format("-{0}", opt).c_str()));
                    }
                }
                if (!handled) {
                    throw std::runtime_error(pn::format("illegal option: -{0}", opt).c_str());
                }
                if (used_value) {
                    break;
                }
            }
        } else {
            bool handled = false;
            try {
                handled = callbacks.argument && callbacks.argument(arg);
            } catch (...) {
                std::throw_with_nested(std::runtime_error(arg.copy().c_str()));
            }
            if (!handled) {
                throw std::runtime_error(
                        pn::format("extra arguments found: {0}", pn::dump(arg, pn::dump_short))
                                .c_str());
            }
        }
    }

    for (; i < argc; ++i) {
        pn::string_view arg     = argv[i];
        bool            handled = false;
        try {
            handled = callbacks.argument && callbacks.argument(arg);
        } catch (...) {
            std::throw_with_nested(std::runtime_error(arg.copy().c_str()));
        }
        if (!handled) {
            throw std::runtime_error(
                    pn::format("extra arguments found: {0}", pn::dump(arg, pn::dump_short))
                            .c_str());
        }
    }
}

template <>
void integer_option<int64_t>(pn::string_view value, int64_t* out) {
    pn_error_code_t error;
    if (!pn::strtoll(value, out, &error)) {
        throw std::runtime_error(pn_strerror(error));
    }
}

template <>
void float_option<double>(pn::string_view value, double* out) {
    pn_error_code_t error;
    if (!pn::strtod(value, out, &error)) {
        throw std::runtime_error(pn_strerror(error));
    }
}

}  // namespace args
}  // namespace sfz
