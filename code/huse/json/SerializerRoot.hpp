// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Serializer.hpp"
#include "SerializerXNode.hpp"
#include "../SerializerRoot.hpp"

namespace huse::json {

using SerializerRoot = huse::SerializerRoot<JsonSerializer>;

} // namespace huse::json