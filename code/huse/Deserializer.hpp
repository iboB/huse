// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "ImValue.hpp"
#include <splat/warnings.h>

namespace huse {

// Yes, yes, we're propagating the warning disable to all includers
// but this class is supposed to be inherited virtually and this triggers the
// stupid and absoltely pointless MSVC warning.
// MSVC, how are we supposed to use virtual inheritance if not like this?
DISABLE_MSVC_WARNING(4250)

class HUSE_API Deserializer {
public:
    virtual ~Deserializer();

    virtual ImValue getRootValue() const = 0;
};

} // namespace huse
