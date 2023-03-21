// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/msg/declare_msg.hpp>

namespace huse {
class Serializer;
class Deserializer;
}

#define HUSE_S_MSG_NAME(tag) I_DNMX_PP_CAT(husePolySerialize_, tag)
#define HUSE_S_MSG(export, type, tag) DYNAMIX_DECLARE_EXPORTED_MSG(export, HUSE_S_MSG_NAME(tag), husePolySerialize, void, (::huse::Serializer&, type));

#define HUSE_D_MSG_NAME(tag) I_DNMX_PP_CAT(husePolyDeserialize_, tag)
#define HUSE_D_MSG(export, type, tag) DYNAMIX_DECLARE_EXPORTED_MSG(export, HUSE_D_MSG_NAME(tag), husePolyDeserialize, void, (::huse::Deserializer&, type&));
