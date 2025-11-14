// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "MsgSerializerValueSerializer.hpp"
#include "CtxObj.hpp"
#include <dynamix/msg/define_msg.hpp>

namespace huse {
class CtxObj;
DYNAMIX_DEFINE_SIMPLE_MSG_EX(Serializer_valueSerializer, unicast, false, nullptr);
} // namespace huse
