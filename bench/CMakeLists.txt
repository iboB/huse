# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
CPMAddPackage(gh:iboB/picobench@2.08)
CPMAddPackage(gh:simdjson/simdjson@3.13.0)
CPMAddPackage(gh:iboB/json-test-data@1.1.0)

if(NOT TARGET Boost::boost)
    CPMAddPackage(
        NAME Boost
        VERSION 1.87.0
        URL https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.xz
        URL_HASH SHA256=7da75f171837577a52bbf217e17f8ea576c7c246e4594d617bfde7fafd408be5
        SYSTEM TRUE
        OPTIONS "BOOST_ENABLE_CMAKE ON"
                "BOOST_INCLUDE_LIBRARIES json"
    )
endif()

if(NOT TARGET Boost::json)
    add_library(Boost::json ALIAS Boost::boost)
endif()

include(icm_cpu_arch)
icm_get_native_arch_flag(nativeArchFlag)

macro(huse_benchmark name)
    set(tgt bench-huse-${name})
    add_executable(${tgt})
    set_target_properties(${tgt} PROPERTIES FOLDER bench)
    target_sources(${tgt} PRIVATE b-${name}.cpp ${ARGN})
    target_compile_options(${tgt} PRIVATE ${nativeArchFlag})
    target_link_libraries(${tgt} PRIVATE
        huse::huse
        Boost::json
        simdjson::simdjson
        test-data::json
        picobench::picobench
    )
    add_custom_target(
        run-${tgt}
        COMMAND ${tgt}
    )
    set_target_properties(run-${tgt} PROPERTIES FOLDER bench)
endmacro()

# huse_benchmark(json-read)
# huse_benchmark(json-parse)
