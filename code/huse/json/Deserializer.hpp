// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../DeserializerRoot.hpp"
#include <string_view>
#include <cstddef>

namespace huse::json {

HUSE_API CtxObj Make_DeserializerCtx(std::string_view str);
HUSE_API CtxObj Make_DeserializerCtx(char* mutableString, size_t len = size_t(-1));

inline DeserializerRoot Make_Deserializer(std::string_view str) {
    return DeserializerRoot(Make_DeserializerCtx(str));
}
inline DeserializerRoot Make_Deserializer(char* mutableString, size_t len = size_t(-1)) {
    return DeserializerRoot(Make_DeserializerCtx(mutableString, len));
}

} // namespace huse::json