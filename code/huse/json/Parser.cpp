// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Parser.hpp"

namespace huse::json {

Parser::Parser(sajson::document&& doc)
    : document(std::move(doc))
{
    if (!document.is_valid()) {
        throw DeserializerException(document.get_error_message_as_cstring());
    }
}

Parser::Parser(std::string_view str)
    : Parser(sajson::parse(
          sajson::single_allocation(),
          sajson::string(str.data(), str.size()))
    )
{}

Parser::Parser(char* str, size_t len)
    : Parser(sajson::parse(
          sajson::single_allocation(),
          sajson::mutable_string_view(len == size_t(-1) ? strlen(str) : len, str))
    )
{}

Parser::~Parser() = default;

ImValue Parser::rootValue() const {
    return document.get_root();
}

} // namespace huse::json
