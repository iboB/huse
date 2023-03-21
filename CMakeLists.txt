# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(huse)

# cpm
include(./get_cpm.cmake)

#######################################
# cmake lib
CPMAddPackage("gh:iboB/icm@1.4.3b#85297e364a87e1f69d52dd1c740ea1c091c27839")

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${icm_SOURCE_DIR}
)

include(icm_dev_mode)
include(icm_add_lib)

#######################################
# options

option(HUSE_STATIC "huse: build as static lib" OFF)
option(HUSE_BUILD_TESTS "huse: build tests" ${ICM_DEV_MODE})
option(HUSE_BUILD_EXAMPLES "huse: build examples" ${ICM_DEV_MODE})

#######################################
# packages
CPMAddPackage("gh:iboB/dynamix@2.0.0-alpha2c#9e3f212132a38c8dcf48769c66d614bc84fe55e9")
CPMAddPackage(gh:iboB/splat@1.2.1)
CPMAddPackage(gh:iboB/mscharconv@1.2.3)
CPMAddPackage(gh:iboB/itlib@1.6.3)

#######################################
# subdirs

add_subdirectory(code/huse)

if(ICM_DEV_MODE)
    add_subdirectory(scratch)
endif()

if(HUSE_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(HUSE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
