// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Serializer.hpp"
#include "SerializerMixin.hpp"
#include "../CtxDomain.hpp"
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/msg_traits.hpp>
#include <dynamix/mutate.hpp>

namespace huse::json {

CtxObj Make_SerializerCtx(std::ostream& out, bool pretty) {
    CtxObj ret;
    mutate(ret, dynamix::add<SerializerMixin>(out, pretty));
    return ret;
}

CtxObj& SerializerMixin::ctx() {
    return huse_ctx_self;
}

DYNAMIX_DEFINE_MIXIN(CtxDomain, SerializerMixin)
    .implements_by<Serializer_valueSerializer>([](SerializerMixin* self) -> ValueSerializer& {
        return *self;
    })
;

}
