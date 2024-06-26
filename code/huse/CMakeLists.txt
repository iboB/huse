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
    PRIVATE
        itlib::itlib
)

include(icm_check_charconv_fp_to_chars)
if(NOT haveCharconvFpToChars)
    message("huse: no charconv floating point support detected. Using mscharconv")
    CPMAddPackage(gh:iboB/mscharconv@1.2.3)
    target_link_libraries(huse PUBLIC msstl::charconv)
    target_compile_definitions(huse PUBLIC -DHUSE_USE_MSCHARCONV=1)
else()
    message("huse: detected charconv floating point support")
endif()
