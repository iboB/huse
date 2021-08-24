icm_add_lib(huse HUSE
    API.h
    Serializer.hpp
    VTableExports.cpp
)

target_include_directories(huse INTERFACE ..)
target_link_libraries(huse PUBLIC
    splat
)
