// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../api.h"
#include "../PojobufDocDeState.hpp"
#include "../ParseTags.hpp"

#include <string_view>
#include <cstddef>
#include <span>

namespace huse::json {

class HUSE_API DeState : public huse::PojobufDocDeState {
public:
    DeState(TagParse,
        std::string_view jsonStr
    );
    DeState(TagParse_withMutableExternalSource,
        std::span<char> mutableStr
    );
    DeState(TagParse_withMutableExternalSource,
        char* mutableStr, size_t size = size_t(-1) // strlen if size is -1
    );
    DeState(TagParse_takeString,
        std::string&& str
    );

    // could be implemented, but it's not needed at this time
    DeState(const DeState&) = delete;
    DeState& operator=(const DeState&) = delete;

    DeState(DeState&&) noexcept = default;
    DeState& operator=(DeState&&) noexcept = default;

    ~DeState();
};

} // namespace huse::json
