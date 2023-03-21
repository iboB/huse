// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <dynamix/msg/declare_msg.hpp>

namespace huse {
class Serializer;
class Deserializer;
template <typename Msg> struct DYNAMIX_FUNC_TRAITS_NAME(husePolySerialize);
template <typename Msg> struct DYNAMIX_FUNC_TRAITS_NAME(husePolyDeserialize);
}

#define HUSE_NO_EXPORT I_DNMX_PP_EMPTY()

#define HUSE_S_MSG_NAME(tag) I_DNMX_PP_CAT(husePolySerialize_, tag)
#define HUSE_S_MSG(export, type, tag) DYNAMIX_DECLARE_EXPORTED_MSG_EX(export, HUSE_S_MSG_NAME(tag), husePolySerialize, \
    ::huse::DYNAMIX_FUNC_TRAITS_NAME(husePolySerialize), void, (::huse::Serializer&, type));

#define HUSE_D_MSG_NAME(tag) I_DNMX_PP_CAT(husePolyDeserialize_, tag)
#define HUSE_D_MSG(export, type, tag) DYNAMIX_DECLARE_EXPORTED_MSG_EX(export, HUSE_D_MSG_NAME(tag), husePolyDeserialize, \
    ::huse::DYNAMIX_FUNC_TRAITS_NAME(husePolyDeserialize), void, (::huse::Deserializer&, type&));

#define HUSE_SD_MSG(export, type, tag) \
    HUSE_S_MSG(export, const type&, tag) \
    HUSE_D_MSG(export, type, tag)
