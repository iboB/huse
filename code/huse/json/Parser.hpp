// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "_sajson/sajson.hpp"
#include <string_view>
#include <cstddef>

namespace huse::json {

struct HUSE_API Parser {
    Parser(sajson::document&& doc);
    Parser(std::string_view str);
    Parser(char* mutableString, size_t len = size_t(-1));

    ~Parser();

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;

    ImValue rootValue() const;

    sajson::document document;
};

} // namespace huse::json
