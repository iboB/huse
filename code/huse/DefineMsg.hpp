// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "DeclareMsg.hpp"
#include "PolyTraits.hpp"
#include <dynamix/msg/define_msg.hpp>

#define HUSE_DEFINE_SERIALIZE_MSG(type, tag) DYNAMIX_DEFINE_MSG(HUSE_S_MSG_NAME(tag), unicast, husePolySerialize, void, (::huse::Serializer&, type))
