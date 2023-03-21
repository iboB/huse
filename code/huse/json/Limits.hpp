// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cstdint>

namespace huse::json {
// json imposed limits (max integer which can be stored in a double)
static inline constexpr int64_t Max_Int64 = 9007199254740992ll;
static inline constexpr int64_t Min_Int64 = -9007199254740992ll;
static inline constexpr uint64_t Max_Uint64 = 9007199254740992ull;
}
