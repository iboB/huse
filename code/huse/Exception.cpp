// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Exception.hpp"
#include <pojobuf/parse_error.to_str.hpp>

namespace huse {

// vtable exports
Exception::~Exception() = default;
SerException::~SerException() = default;
DeException::~DeException() = default;
DeParseException::~DeParseException() = default;
DeTraverseException::~DeTraverseException() = default;

namespace impl {
std::string_view ExceptionWithTrace::getDesc() const noexcept {
    if (trace.empty()) {
        return msg;
    }
    if (!m_descStr.empty()) {
        return m_descStr;
    }

    try {
        m_descStr = trace.describe();
        m_descStr += ": ";
        m_descStr += msg;
        return m_descStr;
    }
    catch (...) {
        return msg;
    }
}
} // namespace impl

std::string_view DeParseException::describe() const noexcept {
    if (!m_descStr.empty()) {
        return m_descStr;
    }

    try {
        m_descStr = pojobuf::to_str(error);
        return m_descStr;
    }
    catch (...) {
        return to_str(error.code);
    }
}

} // namespace huse
