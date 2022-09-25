// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <cassert>

// HUSE_ASSERT_INTERNAL
// asserts for library internal invariants
// if such an assert fails, there's a bug in the library that needs to be fixed
#define HUSE_ASSERT_INTERNAL(x) assert(x)

// HUSE_ASSERT_USAGE
// asserts for the usage of the library
// if such an assert fails, there's a bug in the user code - the library is not
// being used correctly
#define HUSE_ASSERT_USAGE(x, msg) assert((x) && msg)
