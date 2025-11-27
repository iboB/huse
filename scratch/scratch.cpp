// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <iostream>
#include <huse/json/DeserializerRoot.hpp>
#include <huse/Exception.hpp>

//// raw deserializer value
//struct ImValue {
//public:
//    ImValue()
//        : m_type(Type::Null)
//        , m_payload(nullptr)
//        , m_buffer(nullptr)
//    {
//    }
//
//    Type type() const { return m_type; }
//
//    std::string_view getString() const {
//        assert(m_type.isString());
//        const auto ibegin = m_payload[0];
//        const auto iend = m_payload[1];
//        return { m_buffer + ibegin, iend - ibegin };
//    }
//
//
//
//protected:
//    ImValue(Type t, const size_t* payload, const std::byte* buffer)
//        : m_type(t)
//        , m_payload(payload)
//        , m_buffer(buffer)
//    {
//    }
//
//    Type m_type;
//
//    // depending on type this containes either the value
//    // or (for strings and blobs) size and offset in m_buffer
//    const size_t* m_payload;
//
//    // the buffer is used for both strings and blobs
//    static_assert(sizeof(char) == 1);
//    const char* m_buffer;
//};

int main()
{
    constexpr std::string_view json = R"({"ar": [2.3, -5], "val": 5, "b": false})";

    try
    {
        auto d = huse::json::DeserializerRoot::create(json);
        int i;
        d.val(i);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
