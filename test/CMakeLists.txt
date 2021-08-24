include(icm_testing)

CPMAddPackage(
    NAME doctest-lib
    GITHUB_REPOSITORY iboB/doctest-lib
    GIT_TAG 2.4.4
)

macro(huse_test test)
    icm_add_doctest_lib_test(${test} huse ${ARGN})
endmacro()

huse_test(json t-json.cpp)
