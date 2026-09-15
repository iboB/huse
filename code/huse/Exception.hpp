// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "Trace.hpp"
#include <pojobuf/parse_error.hpp>
#include <string_view>
#include <exception>

namespace huse {

class HUSE_API Exception : public std::exception {
public:
    ~Exception();
    virtual std::string_view describe() const noexcept = 0;

    const char* what() const noexcept override final {
        // we promise that it's zero-terminated
        return describe().data();
    }
};

namespace impl {
struct HUSE_API ExceptionWithTrace {
    ExceptionWithTrace(std::string msg, Trace trace = {})
        : msg(std::move(msg)), trace(std::move(trace))
    {}

    std::string msg;
    Trace trace;

    std::string_view getDesc() const noexcept;

private:
    // mutable because we lazily compute it in what or describe
    mutable std::string m_descStr;
};
} // namespace impl

class HUSE_API SerException final : public Exception, public impl::ExceptionWithTrace {
public:
    using impl::ExceptionWithTrace::ExceptionWithTrace;
    ~SerException();
    std::string_view describe() const noexcept override {
        return getDesc();
    }
};

class HUSE_API DeException : public Exception {
public:
    ~DeException();
};

class HUSE_API DeParseException final : public DeException {
public:
    explicit DeParseException(pojobuf::parse_error e)
        : error(std::move(e))
    {}
    ~DeParseException();

    pojobuf::parse_error error;

    std::string_view describe() const noexcept override;
private:
    mutable std::string m_descStr;
};

class HUSE_API DeTraverseException final : public DeException, public impl::ExceptionWithTrace {
public:
    using impl::ExceptionWithTrace::ExceptionWithTrace;
    ~DeTraverseException();
    std::string_view describe() const noexcept override {
        return getDesc();
    }
};

} // namespace huse
