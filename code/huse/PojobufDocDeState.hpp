// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "PojobufDeState.hpp"

#include <pojobuf/document.hpp>

#include <string_view>
#include <cstddef>
#include <span>

namespace huse {

class HUSE_API PojobufDocDeState : public huse::PojobufDeState {
public:
    using DocType = pojobuf::document<std::string>;
    PojobufDocDeState() = default;
    explicit PojobufDocDeState(DocType doc)
        : m_document(std::move(doc))
    {}

    PojobufDocDeState(const PojobufDocDeState&) = delete;
    PojobufDocDeState& operator=(const PojobufDocDeState&) = delete;

    PojobufDocDeState(PojobufDocDeState&&) noexcept = default;
    PojobufDocDeState& operator=(PojobufDocDeState&&) noexcept = default;

    ~PojobufDocDeState();

    void initRoot() {
        push(m_document.root());
    }

protected:
    DocType m_document;
};

} // namespace huse
