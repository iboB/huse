// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "CtxFwd.hpp"
#include <trex/facets/facets.hpp>

namespace huse {

using Ctx = trex::facets<CtxDomain, trex::default_facet_container, trex::lock::fast>;

} // namespace huse
