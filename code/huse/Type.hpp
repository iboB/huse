// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>

namespace huse {

struct Type {
public:
    enum Value : uint8_t {
        Null   = 0,
        String = 1,
        Object = 2,
        Array  = 3,

        True =  0b01'0000,
        False = 0b10'0000,

        Integer = 0b0100'0000,
        Float   = 0b1000'0000,
    };

    static inline constexpr uint8_t Boolean_Mask = True | False;
    static inline constexpr uint8_t Number_Mask = Integer | Float;

    // intentionally implicit
    constexpr Type(Value t) : m_t(t) {}

    constexpr bool isNull() const { return m_t == Null; }
    constexpr bool isString() const { return m_t == String; }

    constexpr bool isTrue() const { return m_t == True; }
    constexpr bool isFalse() const { return m_t == False; }
    constexpr bool isBoolean() const { return (m_t & Boolean_Mask) != 0; }

    constexpr bool isInteger() const { return m_t == Integer; }
    constexpr bool isFloat() const { return m_t == Float; }
    constexpr bool isNumber() const { return (m_t & Number_Mask) != 0; }

    constexpr bool isObject() const { return m_t == Object; }
    constexpr bool isArray() const { return m_t == Array; }

    constexpr bool operator==(const Type& other) const { return m_t == other.m_t; }
    constexpr bool operator!=(const Type& other) const { return m_t != other.m_t; }
private:
    Value m_t;
};

}
