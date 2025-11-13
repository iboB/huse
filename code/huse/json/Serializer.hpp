// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../Serializer.hpp"
#include <iosfwd>

namespace huse::json {

HUSE_API std::shared_ptr<Serializer> Make_SerializerPtr(std::ostream& out, bool pretty = false);

inline SerializerRoot Make_Serializer(std::ostream& out, bool pretty = false) {
    return SerializerRoot(Make_SerializerPtr(out, pretty));
}

} // namespace huse::json
