// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Deserializer.hpp"
#include "MsgDeserializerRoot.hpp"
#include "CtxObj.hpp"

namespace huse {

class DeserializerRoot : public DeserializerNode {
    CtxObj m_ctxobj;
public:
    explicit DeserializerRoot(CtxObj&& d)
        : DeserializerNode(m_ctxobj, Deserializer_root::call(d))
        , m_ctxobj(std::move(d))
    {}
};

} // namespace huse
