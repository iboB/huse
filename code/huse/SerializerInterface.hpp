// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "DeclareMsg.hpp"

namespace huse {
HUSE_SERIALIZE_MSG(HUSE_API, bool, bool);
HUSE_SERIALIZE_MSG(HUSE_API, short, short);
HUSE_SERIALIZE_MSG(HUSE_API, unsigned short, ushort);
}

#define huse_BasicInterface husePolySerialize_bool_msg & husePolySerialize_short_msg & husePolySerialize_ushort_msg

