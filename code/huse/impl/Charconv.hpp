// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

#if HUSE_USE_MSCHARCONV
#   include <msstl/charconv.hpp>
#	define HUSE_CHARCONV_NAMESPACE msstl
#else
#   include <charconv>
#	define HUSE_CHARCONV_NAMESPACE std
#endif
