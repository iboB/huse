// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include <cstdint>

namespace huse
{

class Context
{
public:
    using val_t = uintptr_t;

    Context(val_t val = 0) : m_val(val) {}

    val_t i() const { return m_val; }

private:
    val_t m_val;
};

}
