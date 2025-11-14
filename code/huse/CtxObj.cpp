// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "CtxObj.hpp"
#include "CtxDomain.hpp"

namespace huse {

CtxObj::CtxObj()
    : dynamix::object(dynamix::g::get_domain<CtxDomain>())
{}

} // namespace huse
