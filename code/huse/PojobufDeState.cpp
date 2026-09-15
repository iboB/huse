// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "PojobufDeState.hpp"
#include "Trace.hpp"
#include "Exception.hpp"

namespace huse {

PojobufDeState::~PojobufDeState() {
    assert(m_stack.size() == 0);
}

Trace PojobufDeState::getTrace() const {
    if (m_stack.empty()) {
        return {};
    }

    Trace trace;
    trace.topType = topType();

    trace.entries.reserve(m_stack.size() - 1);
    for(auto it = m_stack.begin(); it != m_stack.end() - 1; ++it) {
        auto& se = *it;
        auto& te = trace.entries.emplace_back();
        const auto i = se.curIndex - 1;
        te.index = i;

        auto& value = se.value;
        assert(value.type().is_compound());

        if (value.type().is_object()) {
            if (i < value.compound_length()) {
                te.key = value.object_key_at(i);
            }
            else {
                te.key = m_lastMissingKey;
            }
        }
    }
    return trace;
}

void PojobufDeState::throwException(std::string msg) const {
    throw DeTraverseException(std::move(msg), getTrace());
}

} // namespace huse
