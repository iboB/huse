// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include <dynamix/msg/declare_msg.hpp>

namespace huse {
class CtxObj;
class ValueSerializer;
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, Serializer_valueSerializer, ValueSerializer&(CtxObj&));
} // namespace huse
