// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Deserializer.hpp"
#include "DeserializerMixin.hpp"
#include "../CtxDomain.hpp"
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/msg_traits.hpp>
#include <dynamix/mutate.hpp>

namespace huse::json {

CtxObj Make_DeserializerCtx(std::string_view str) {
    CtxObj ret;
    mutate(ret, dynamix::add<DeserializerMixin>(str));
    return ret;
}

CtxObj Make_DeserializerCtx(char* mutableString, size_t len) {
    CtxObj ret;
    mutate(ret, dynamix::add<DeserializerMixin>(mutableString, len));
    return ret;
}

DYNAMIX_DEFINE_MIXIN(CtxDomain, DeserializerMixin)
    .implements_by<Deserializer_root>([](const DeserializerMixin* self) {
        return self->rootValue();
    })
;

} // namespace huse::json