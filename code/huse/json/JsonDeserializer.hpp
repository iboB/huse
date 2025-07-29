// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../Deserializer.hpp"
#include <dynamix/declare_mixin.hpp>
#include <string_view>
#include <cstddef>
// #include <dynamix/common_mixin_init.hpp>

namespace huse::json {
DYNAMIX_DECLARE_EXPORTED_MIXIN(HUSE_API, struct JsonDeserializer);

//struct HUSE_API Deserializer : public dynamix::common_mixin_init<JsonDeserializer> {
//    std::ostream& out;
//    bool pretty;
//    Deserializer(std::ostream& out, bool pretty = false) : out(out), pretty(pretty) {}
//    virtual void do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) final override;
//};

HUSE_API Deserializer Make_DeserializerObj(std::string_view str);
HUSE_API Deserializer Make_DeserializerObj(char* mutableString, size_t len = size_t(-1));

inline DeserializerRoot Make_Deserializer(std::string_view str) {
    return DeserializerRoot(Make_DeserializerObj(str));
}
inline DeserializerRoot Make_Deserializer(char* mutableString, size_t len = size_t(-1)) {
    return DeserializerRoot(Make_DeserializerObj(mutableString, len));
}

}
