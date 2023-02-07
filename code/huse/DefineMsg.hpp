// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "DeclareMsg.hpp"
#include <dynamix/define_message.hpp>

#define HUSE_DEFINE_SERIALIZE_MSG(tag) DYNAMIX_DEFINE_MESSAGE(HUSE_S_MSG_NAME(tag))
#define HUSE_DEFINE_SERIALIZE_MSG_IMPL(type, tag) DYNAMIX_DEFINE_MESSAGE_1_WITH_DEFAULT_IMPL(void, HUSE_S_MSG_NAME(tag), type)
