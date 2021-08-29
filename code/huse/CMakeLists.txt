icm_add_lib(huse HUSE
    API.h

    impl/UniqueStack.hpp

    fwd.hpp

    Serializer.hpp
    Deserializer.hpp
    VTableExports.cpp

    JsonSerializer.hpp
    JsonSerializer.cpp
    JsonDeserializer.hpp
    JsonDeserializer.cpp
)

target_include_directories(huse INTERFACE ..)
target_link_libraries(huse
    PUBLIC
        splat
    PRIVATE
        mscharconv
)
