// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "Type.hpp"
#include <optional>
#include <cstdint>
#include <string>
#include <vector>

namespace huse {

class HUSE_API Trace {
public:
    struct Entry {
        uint32_t index;
        std::optional<std::string> key;
    };
    std::vector<Entry> entries;
    Type topType = Type::Undefined;

    bool empty() const noexcept {
        return entries.empty() && topType.isUndefined();
    }

    std::string describe() const;
};

} // namespace huse
