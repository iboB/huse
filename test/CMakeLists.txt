# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/doctest-util@0.1.3)

macro(huse_test test)
    add_doctest_lib_test(${test} huse t-${test}.cpp ${ARGN})
endmacro()

huse_test(json)
#huse_test(poly)
#huse_test(helpers)
