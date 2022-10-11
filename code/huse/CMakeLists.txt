# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
icm_add_lib(huse HUSE
    API.h

    impl/UniqueStack.hpp

    Fwd.hpp

    Serializer.hpp
    Deserializer.hpp
    VTableExports.cpp
    Exception.hpp

    json/Serializer.hpp
    json/Serializer.cpp
    json/Deserializer.hpp
    json/Deserializer.cpp
    json/_sajson/sajson.hpp

    helpers/StdVector.hpp
)
add_library(huse::huse ALIAS huse)

target_include_directories(huse INTERFACE ..)
target_link_libraries(huse
    PUBLIC
        splat::splat
    PRIVATE
        msstl::charconv
        itlib::itlib
)
