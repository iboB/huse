// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once
#include <cassert>

namespace huse::impl
{

class UniqueStack
{
    UniqueStack* m_parent;
    bool m_hasActiveSubObject = false;
public:
    UniqueStack(UniqueStack* parent)
        : m_parent(parent)
    {
        if (!m_parent) return;
        assert(!m_parent->m_hasActiveSubObject);
        m_parent->m_hasActiveSubObject = true;
    }
    ~UniqueStack() {
        if (!m_parent) return;
        assert(m_parent->m_hasActiveSubObject);
        m_parent->m_hasActiveSubObject = false;
    }
};

}
