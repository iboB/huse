// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include <dynamix/object.hpp>
#include <dynamix/object_of.hpp>

namespace huse {
class DeserializerNode;
class HUSE_API Deserializer : public dynamix::object {
public:
    Deserializer();

    // implemented in Deserializer.hpp
    DeserializerNode node();
    DeserializerNode root();

    static Deserializer* of(void* mixin) {
        return static_cast<Deserializer*>(dynamix::object_of(mixin));
    }
    static const Deserializer* of(const void* mixin) {
        return static_cast<const Deserializer*>(dynamix::object_of(mixin));
    }
};

#define huse_d_self ::huse::Deserializer::of(this)
}
