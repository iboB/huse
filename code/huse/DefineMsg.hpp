// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "DeclareMsg.hpp"
#include "PolyTraits.hpp"
#include <dynamix/msg/define_msg.hpp>

#define HUSE_DEFINE_S_MSG_EX(type, tag, clash, default_impl) DYNAMIX_DEFINE_MSG_EX(HUSE_S_MSG_NAME(tag), unicast, clash, default_impl, husePolySerialize, void, (::huse::Serializer&, type))
#define HUSE_DEFINE_D_MSG_EX(type, tag, clash, default_impl) DYNAMIX_DEFINE_MSG_EX(HUSE_D_MSG_NAME(tag), unicast, clash, default_impl, husePolyDeserialize, void, (::huse::Deserializer&, type))

#define HUSE_DEFINE_S_MSG(type, tag) HUSE_DEFINE_S_MSG_EX(type, tag, true, nullptr)
#define HUSE_DEFINE_D_MSG(type, tag) HUSE_DEFINE_D_MSG_EX(type, tag, true, nullptr)

#define HUSE_DEFINE_SD_MSG(type, tag) \
    HUSE_DEFINE_S_MSG(const type&, tag); \
    HUSE_DEFINE_D_MSG(type&, tag)
