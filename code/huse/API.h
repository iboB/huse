// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
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
