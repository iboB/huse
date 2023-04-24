# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
include(icm_testing)

CPMAddPackage(gh:iboB/doctest-util@0.0.5)

macro(huse_test test)
    add_doctest_lib_test(${test} huse ${ARGN})
endmacro()

huse_test(json t-json.cpp)
huse_test(poly t-poly.cpp)
huse_test(helpers t-helpers.cpp)
