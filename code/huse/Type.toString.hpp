// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Type.hpp"
#include <string_view>

namespace huse {

constexpr std::string_view Type_toString(Type t) {
    using enum Type::Value;
    switch (t.value()) {
    case Undefined: return "undefined";
    case Null:      return "null";
    case Boolean:   return "boolean";
    case String:    return "string";
    case Array:     return "array";
    case Object:    return "object";
    case Custom:    return "user-defined";
    case Integer:   return "integer";
    case Float:     return "float";
    default: return "???";
    }
}

} // namespace huse
