// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "DeState.hpp"
#include "../Exception.hpp"
#include <pojobuf/document_parse.hpp>
#include <pojobuf/json/parser.hpp>
#include <cstring>

namespace huse::json {

namespace {
pojobuf::document<std::string> docOrThrow(
    itlib::expected<pojobuf::document<std::string>, pojobuf::parse_error> res
) {
    if (!res) {
        throw DeParseException(std::move(res).error());
    }
    return std::move(res).value();
}
} // namespace

using Parser = pojobuf::json::parser_charconv_num;
using namespace pojobuf;

DeState::~DeState() = default;

DeState::DeState(TagParse, std::string_view jsonStr) {
    m_document = docOrThrow(document_parse<Parser, std::string>(jsonStr));
}
DeState::DeState(TagParse_withMutableExternalSource, std::span<char> mutableStr) {
    m_document = docOrThrow(document_parse_with<
        Parser,
        parse_alloc_strategy::use_external_mutable_source,
        std::string
    >(mutableStr));
}

DeState::DeState(TagParse_withMutableExternalSource, char* mutableStr, size_t size)
    : DeState(
        Parse_withMutableExternalSource,
        std::span<char>(mutableStr, size == size_t(-1) ? std::strlen(mutableStr) : size)
    )
{}

DeState::DeState(TagParse_takeString, std::string&& str) {
    m_document = docOrThrow(document_parse_with<
        Parser,
        parse_alloc_strategy::take_source
    >(std::move(str)));
}

} // namespace huse::json
