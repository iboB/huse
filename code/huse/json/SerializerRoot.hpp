// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "Serializer.hpp"
#include "../Serializer.hpp"

namespace huse::json {

class SerializerRoot : public JsonSerializer, public SerializerNode {
public:
    SerializerRoot(std::ostream& out, bool pretty = false)
        : JsonSerializer(out, pretty)
        , SerializerNode(*this, nullptr)
    {}
};

} // namespace huse::json