# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
icm_add_lib(huse HUSE)
add_library(huse::huse ALIAS huse)

target_sources(huse
    INTERFACE FILE_SET HEADERS FILES
        huse/api.h
    PRIVATE
        huse/PojobufDeState.cpp
        huse/PojobufDocDeState.cpp
        huse/ISerState.cpp
        huse/Trace.cpp
        huse/Exception.cpp

        huse/json/DeState.cpp
        huse/json/SerState.cpp
)

target_include_directories(huse INTERFACE ..)
target_link_libraries(huse
    PUBLIC
        splat::splat
        itlib::itlib
        pojobuf::pojobuf
)
