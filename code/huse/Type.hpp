// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace huse {

struct Type
{
public:
    enum Value : int
    {
        True = 0b01,
        False = 0b10,
        Boolean = 0b11,
        Integer = 0b0100,
        Float = 0b1000,
        Number = 0b1100,
        String = 0b10000,
        Object = 0b100000,
        Array = 0b1000000,
        Null = 0b10000000,
    };

    Type(Value t) : m_t(t) {}

    bool is(Value mask) const { return m_t & mask; }
private:
    Value m_t;
};

}
