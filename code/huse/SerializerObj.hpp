// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include <dynamix/object.hpp>
#include <dynamix/object_of.hpp>

namespace huse {
class SerializerNode;
class HUSE_API Serializer : public dynamix::object {
public:
    Serializer();

    // implemented in Serializer.hpp
    SerializerNode node();
    SerializerNode root();

    static Serializer* of(void* mixin) {
        return static_cast<Serializer*>(dynamix::object_of(mixin));
    }
    static const Serializer* of(const void* mixin) {
        return static_cast<const Serializer*>(dynamix::object_of(mixin));
    }
};

#define huse_s_self ::huse::Serializer::of(this)

}
