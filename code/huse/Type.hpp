// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>

namespace huse {

struct Type {
public:
    enum Value : uint8_t {
        Undefined = 0, // uninitialized node
        Null,
        Boolean,
        String,
        Array,
        Object,

        Custom,

        Integer = 0b0100'0000,
        Float   = 0b1000'0000,
    };
    static_assert(Custom < Integer, "Non-masked types should be below masked types");

    static inline constexpr uint8_t Number_Mask = Integer | Float;

    // intentionally implicit
    constexpr Type(Value t) : m_t(t) {}

    constexpr bool isUndefined() const { return m_t == Undefined; }

    constexpr bool isNull() const { return m_t == Null; }
    constexpr bool isString() const { return m_t == String; }

    constexpr bool isBoolean() const { return m_t == Boolean; }

    constexpr bool isInteger() const { return m_t == Integer; }
    constexpr bool isFloat() const { return m_t == Float; }
    constexpr bool isNumber() const { return (m_t & Number_Mask) != 0; }

    constexpr bool isArray() const { return m_t == Array; }
    constexpr bool isObject() const { return m_t == Object; }

    constexpr bool operator==(const Type& other) const = default;
    constexpr bool operator!=(const Type& other) const = default;

    constexpr Value value() const { return m_t; }
private:
    Value m_t;
};

}
