// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "OpenTags.hpp"
#include "ImValue.hpp"

namespace huse {

class HUSE_API Deserializer {
public:
    virtual ~Deserializer();

    virtual ImValue getRootValue() const = 0;

    struct XNode {};
    struct XArray {};
    struct XObject {};
};

} // namespace huse