// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Serializer.hpp"
#include "MsgSerializerValueSerializer.hpp"
#include "CtxObj.hpp"

namespace huse {

class SerializerRoot : public SerializerNode {
    CtxObj m_ctxobj;
public:
    SerializerRoot(CtxObj&& s)
        : SerializerNode(Serializer_valueSerializer::call(s), nullptr)
        , m_ctxobj(std::move(s))
    {}
};

} // namespace huse
