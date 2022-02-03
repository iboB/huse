include(icm_testing)

CPMAddPackage(gh:iboB/doctest-lib@2.4.6)

macro(huse_test test)
    icm_add_doctest_lib_test(${test} huse ${ARGN})
endmacro()

huse_test(json t-json.cpp)
