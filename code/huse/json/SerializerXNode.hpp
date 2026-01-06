// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../SerializerNode.hpp"
#include "Serializer.hpp"

namespace huse::json {

struct SerializerXNode {
    void raw(std::string_view json) {
        static_cast<SerializerNode<JsonSerializer>*>(this)->_s().writeRawJson(json);
    }
};

} // namespace huse::json
