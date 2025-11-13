// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../Deserializer.hpp"
#include <string_view>
#include <cstddef>

namespace huse::json {

HUSE_API std::shared_ptr<Deserializer> Make_DeserializerPtr(std::string_view str);
HUSE_API std::shared_ptr<Deserializer> Make_DeserializerPtr(char* mutableString, size_t len = size_t(-1));

inline DeserializerRoot Make_Deserializer(std::string_view str) {
    return DeserializerRoot(Make_DeserializerPtr(str));
}
inline DeserializerRoot Make_Deserializer(char* mutableString, size_t len = size_t(-1)) {
    return DeserializerRoot(Make_DeserializerPtr(mutableString, len));
}

} // namespace huse::json