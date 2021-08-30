icm_add_lib(huse HUSE
    API.h

    impl/UniqueStack.hpp

    Fwd.hpp

    Serializer.hpp
    SerializerException.hpp
    Deserializer.hpp
    VTableExports.cpp

    json/Serializer.hpp
    json/Serializer.cpp
    json/Deserializer.hpp
    json/Deserializer.cpp
    json/_sajson/sajson.hpp

    helpers/StdVector.hpp
)

target_include_directories(huse INTERFACE ..)
target_link_libraries(huse
    PUBLIC
        splat
    PRIVATE
        mscharconv
)
