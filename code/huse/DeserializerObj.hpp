// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include <dynamix/object.hpp>

namespace huse {
class DeserializerNode;
class HUSE_API Deserializer : public dynamix::object {
public:
    Deserializer();

    // implemented in Deserializer.hpp
    DeserializerNode node();
    DeserializerNode root();
};
}
