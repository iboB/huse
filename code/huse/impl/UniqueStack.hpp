// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once
#include "Assert.hpp"

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
        HUSE_ASSERT_USAGE(!m_parent->m_hasActiveSubObject, "multi subobj");
        m_parent->m_hasActiveSubObject = true;
    }
    ~UniqueStack()
    {
        HUSE_ASSERT_USAGE(!m_hasActiveSubObject, "parent before child");
        if (!m_parent) return;
        HUSE_ASSERT_INTERNAL(m_parent->m_hasActiveSubObject);
        m_parent->m_hasActiveSubObject = false;
    }

    UniqueStack(const UniqueStack&) = delete;
    UniqueStack& operator=(const UniqueStack&) = delete;
    UniqueStack(UniqueStack&& other)
        : m_parent(other.m_parent)
        , m_hasActiveSubObject(other.m_hasActiveSubObject)
    {
        other.m_parent = nullptr;
        other.m_hasActiveSubObject = false;
    }
    UniqueStack& operator=(UniqueStack&&) = delete;
};

}
