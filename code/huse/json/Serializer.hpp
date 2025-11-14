// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../SerializerRoot.hpp"
#include <iosfwd>

namespace huse::json {

HUSE_API CtxObj Make_SerializerCtx(std::ostream& out, bool pretty = false);

inline SerializerRoot Make_Serializer(std::ostream& out, bool pretty = false) {
    return SerializerRoot(Make_SerializerCtx(out, pretty));
}

} // namespace huse::json
