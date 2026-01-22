// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "Parser.hpp"
#include "../Deserializer.hpp"

namespace huse::json {

class HUSE_API JsonDeserializer : virtual public Deserializer, private Parser {
public:
    using Parser::Parser;
    ~JsonDeserializer();

    const Parser& jsonParser() const { return *this; }

    virtual ImValue getRootValue() const override {
        return rootValue();
    }
};

} // namespace huse::json