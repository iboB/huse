// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "Parser.hpp"
#include <dynamix/declare_mixin.hpp>

namespace huse::json {

struct DeserializerMixin : public Parser {
    using Parser::Parser;
};

DYNAMIX_DECLARE_EXPORTED_MIXIN(HUSE_API, DeserializerMixin);

} // namespace huse::json
