// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "JsonValueSerializer.hpp"
#include <dynamix/declare_mixin.hpp>

namespace huse::json {

struct HUSE_API SerializerMixin final : public JsonValueSerializer {
    using JsonValueSerializer::JsonValueSerializer;
    virtual CtxObj& ctx() override;
};

DYNAMIX_DECLARE_EXPORTED_MIXIN(HUSE_API, SerializerMixin);

} // namespace huse::json
