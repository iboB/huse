# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(huse)

# cpm
include(./get_cpm.cmake)

#######################################
# cmake lib
CPMAddPackage(gh:iboB/icm@1.5.5)

list(APPEND CMAKE_MODULE_PATH
    "${icm_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

include(icm_dev_mode)
include(icm_add_lib)

#######################################
# options

option(BUILD_SHARED_LIBS "${PROJECT_NAME}: build shared libraries" ON)
option(HUSE_BUILD_TESTS "huse: build tests" ${ICM_DEV_MODE})
option(HUSE_BUILD_EXAMPLES "huse: build examples" ${ICM_DEV_MODE})

#######################################
# packages
CPMAddPackage(gh:iboB/splat@1.3.3)
CPMAddPackage(gh:iboB/itlib@1.11.7)
CPMAddPackage(gh:iboB/dynamix@2.0.4)

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
