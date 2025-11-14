// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "ImValue.hpp"
#include <dynamix/msg/declare_msg.hpp>

namespace huse {
class CtxObj;
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, Deserializer_root, ImValue(const CtxObj&));
} // namespace huse
