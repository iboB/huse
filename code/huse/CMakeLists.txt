icm_add_lib(huse HUSE
    API.h

    impl/UniqueStack.hpp

    Serializer.hpp
    VTableExports.cpp

    JsonSerializer.hpp
    JsonSerializer.cpp
)

target_include_directories(huse INTERFACE ..)
target_link_libraries(huse PUBLIC
    splat
    mscharconv
)
