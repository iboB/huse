# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(huse)

# cpm
include(./get_cpm.cmake)

#######################################
# cmake lib
CPMAddPackage(gh:iboB/icm@1.4.4)

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
CPMAddPackage(gh:iboB/dynamix@2.0.2)
CPMAddPackage(gh:iboB/splat@1.3.2)
CPMAddPackage(gh:iboB/itlib@1.10.3)
CPMAddPackage(gh:iboB/mscharconv@1.2.3)

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
