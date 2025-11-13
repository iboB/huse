// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../Deserializer.hpp"
#include "_sajson/sajson.hpp"
#include <string_view>
#include <cstddef>

namespace huse::json {

class HUSE_API JsonDeserializer : public Deserializer {
public:
    JsonDeserializer(std::string_view str);
    JsonDeserializer(char* mutableString, size_t len = size_t(-1));
    ~JsonDeserializer();

protected:
    JsonDeserializer(sajson::document&& doc);
    virtual impl::RawDValue rootValue() const override;
    sajson::document m_document;
};

} // namespace huse::json
