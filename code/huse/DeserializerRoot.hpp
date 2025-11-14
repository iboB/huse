// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Deserializer.hpp"

namespace huse {

class HUSE_API Deserializer {
public:
    virtual ~Deserializer();
    DeserializerNode root() {
        return DeserializerNode(*this, rootValue());
    }
protected:
    virtual ImValue rootValue() const = 0;
    friend class DeserializerRoot;
};

class DeserializerRoot : public DeserializerNode {
    std::shared_ptr<Deserializer> m_deserializerObject;
public:
    explicit DeserializerRoot(std::shared_ptr<Deserializer> d)
        : DeserializerNode(*d, d->rootValue())
        , m_deserializerObject(std::move(d))
    {}
};

} // namespace huse
