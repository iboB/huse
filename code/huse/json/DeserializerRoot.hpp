// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Deserializer.hpp"
#include "../DeserializerRoot.hpp"

namespace huse::json {

using DeserializerRoot = huse::DeserializerRoot<JsonDeserializer>;

} // namespace huse::json
