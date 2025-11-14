// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include <dynamix/object.hpp>
#include <dynamix/object_of.hpp>


namespace huse {

class HUSE_API CtxObj : dynamix::object {
public:
    CtxObj();

    static CtxObj& of(void* mixin) {
        return *static_cast<CtxObj*>(dynamix::object_of(mixin));
    }
    static const CtxObj& of(const void* mixin) {
        return *static_cast<const CtxObj*>(dynamix::object_of(mixin));
    }
};

#define huse_ctx_self ::huse::CtxObj::of(this)

} // namespace huse
