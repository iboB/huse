// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/declare_message.hpp>

#define HUSE_S_MSG_NAME(tag) I_DYNAMIX_PP_CAT(husePolySerialize_, tag)
#define HUSE_SERIALIZE_MSG(export, type, tag) DYNAMIX_EXPORTED_MESSAGE_1_OVERLOAD(export, HUSE_S_MSG_NAME(tag), void, husePolySerialize, type, val);
