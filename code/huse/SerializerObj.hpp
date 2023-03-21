// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include <dynamix/object.hpp>

namespace huse {
class SerializerNode;
class HUSE_API Serializer : public dynamix::object {
public:
    Serializer();

    // implemented in Serializer.hpp
    SerializerNode node();
    SerializerNode root();
};
}
