// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "DeserializerNode.hpp"

namespace huse {

template <typename SDeserializer>
class DeserializerRoot : public SDeserializer, public DeserializerNode<SDeserializer> {
public:
    template <typename... Args>
    DeserializerRoot(Args&&... args)
        : SDeserializer(std::forward<Args>(args)...)
        , DeserializerNode<SDeserializer>(this->SDeserializer::getRootValue())
    {}
};

} // namespace huse
