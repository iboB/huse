// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "MsgDeserializerRoot.hpp"
#include "CtxObj.hpp"
#include <dynamix/msg/define_msg.hpp>

namespace huse {
class CtxObj;
DYNAMIX_DEFINE_SIMPLE_MSG_EX(Deserializer_root, unicast, false, nullptr);
} // namespace huse
