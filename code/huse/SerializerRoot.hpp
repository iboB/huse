// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Serializer.hpp"

namespace huse {

template <typename SSerializer>
class SerializerRoot : public SSerializer, public SerializerNode {
public:
    template <typename... Args>
    SerializerRoot(Args&&... args)
        : SSerializer(std::forward<Args>(args)...)
        , SerializerNode(*this, nullptr)
    {}
};

} // namespace huse
