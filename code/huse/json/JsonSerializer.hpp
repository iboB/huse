// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../SerializerObj.hpp"
#include <dynamix/declare_mixin.hpp>
#include <iosfwd>
// #include <dynamix/common_mixin_init.hpp>

namespace huse::json {
DYNAMIX_DECLARE_EXPORTED_MIXIN(HUSE_API, struct JsonSerializer);

//struct HUSE_API Serializer : public dynamix::common_mixin_init<JsonSerializer> {
//    std::ostream& out;
//    bool pretty;
//    Serializer(std::ostream& out, bool pretty = false) : out(out), pretty(pretty) {}
//    virtual void do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) final override;
//};

HUSE_API Serializer Make_Serializer(std::ostream& out, bool pretty = false);
}
