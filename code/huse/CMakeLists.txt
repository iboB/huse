# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
icm_add_lib(huse HUSE
    API.h

    impl/UniqueStack.hpp

    Domain.hpp
    Domain.cpp

    Fwd.hpp

    Serializer.hpp
    SerializerInterface.hpp
    SerializerInterface.cpp
    Deserializer.hpp
    DeserializerInterface.hpp
    DeserializerInterface.cpp
    VTableExports.cpp
    Exception.hpp

    json/Serializer.hpp
    json/JsonSerializer.hpp
    json/JsonSerializer.cpp
    json/Deserializer.hpp
    json/JsonDeserializer.cpp
    json/_sajson/sajson.hpp

    helpers/StdVector.hpp
)
add_library(huse::huse ALIAS huse)

target_include_directories(huse INTERFACE ..)
target_link_libraries(huse
    PUBLIC
        dynamix
        splat::splat
        msstl::charconv
    PRIVATE
        itlib::itlib
)
