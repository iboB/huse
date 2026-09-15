// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <splat/symbol_export.h>

#if HUSE_SHARED
#   if BUILDING_HUSE
#       define HUSE_API SYMBOL_EXPORT
#   else
#       define HUSE_API SYMBOL_IMPORT
#   endif
#else
#   define HUSE_API
#endif
