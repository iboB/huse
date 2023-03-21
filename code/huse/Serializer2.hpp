// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"

#include "impl/UniqueStack.hpp"

#include <dynamix/object.hpp>

#include <string_view>
#include <optional>
#include <iosfwd>

namespace huse
{

class HUSE_API Serializer : public dynamix::object {
public:
    Serializer();
};

} // namespace huse
