// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "JsonDeserializer.hpp"
#include "Deserializer.hpp"

namespace huse::json {

JsonDeserializer::JsonDeserializer(sajson::document&& doc)
    : m_document(std::move(doc))
{
    if (!m_document.is_valid()) {
        throw DeserializerException(m_document.get_error_message_as_cstring());
    }
}

JsonDeserializer::JsonDeserializer(std::string_view str)
    : JsonDeserializer(sajson::parse(
          sajson::single_allocation(),
          sajson::string(str.data(), str.size()))
    )
{}

JsonDeserializer::JsonDeserializer(char* str, size_t len)
    : JsonDeserializer(sajson::parse(
          sajson::single_allocation(),
          sajson::mutable_string_view(len == size_t(-1) ? strlen(str) : len, str))
    )
{}

JsonDeserializer::~JsonDeserializer() = default;

impl::RawDValue JsonDeserializer::rootValue() const {
    return m_document.get_root();
}

std::shared_ptr<Deserializer> Make_DeserializerPtr(std::string_view str) {
    return std::make_shared<JsonDeserializer>(str);
}
std::shared_ptr<Deserializer> Make_DeserializerPtr(char* str, size_t len) {
    return std::make_shared<JsonDeserializer>(str, len);
}

} // namespace huse::json
