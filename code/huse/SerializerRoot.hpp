// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Serializer.hpp"

namespace huse {

template <typename SSerializer>
class SerializerRoot : private SSerializer, public SerializerNode<SSerializer> {
public:
    template <typename... Args>
    SerializerRoot(Args&&... args)
        : SSerializer(std::forward<Args>(args)...)
        , SerializerNode<SSerializer>((SSerializer&)*this)
    {}
};

} // namespace huse
