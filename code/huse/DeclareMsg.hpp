// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/msg/declare_msg.hpp>

namespace huse {
class Serializer;
}

#define HUSE_S_MSG_NAME(tag) I_DNMX_PP_CAT(husePolySerialize_, tag)
#define HUSE_SERIALIZE_MSG(export, type, tag) DYNAMIX_DECLARE_EXPORTED_MSG(export, HUSE_S_MSG_NAME(tag), husePolySerialize, void, (::huse::Serializer&, type));
