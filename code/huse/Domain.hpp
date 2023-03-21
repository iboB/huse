// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include <dynamix/declare_domain.hpp>

namespace huse {
struct SerializerDomain;
DYNAMIX_DECLARE_EXPORTED_DOMAIN(HUSE_API, SerializerDomain);
struct DeserializerDomain;
DYNAMIX_DECLARE_EXPORTED_DOMAIN(HUSE_API, DeserializerDomain);
}
