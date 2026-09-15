// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace huse {

class TagParse {};
class TagParse_withMutableExternalSource {};
class TagParse_takeString {};

inline constexpr TagParse Parse;
inline constexpr TagParse_withMutableExternalSource Parse_withMutableExternalSource;
inline constexpr TagParse_takeString Parse_takeString;

} // namespace huse