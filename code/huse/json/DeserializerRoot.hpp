// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "Parser.hpp"
#include "../Deserializer.hpp"
#include <string_view>
#include <cstddef>

namespace huse::json {

class DeserializerRoot : public DeserializerNode {
    Parser m_parser;
    DeserializerRoot(Parser&& p)
        : DeserializerNode(p.rootValue())
        , m_parser(std::move(p))
    {}
public:
    static DeserializerRoot create(std::string_view str) {
        return DeserializerRoot(Parser(str));
    }
    static DeserializerRoot create(char* mutableString, size_t len = size_t(-1)) {
        return DeserializerRoot(Parser(mutableString, len));
    }
};

} // namespace huse::json