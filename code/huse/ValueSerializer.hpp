// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include <string_view>
#include <string>
#include <cstddef>
#include <optional>

namespace huse {
class CtxObj;

class HUSE_API ValueSerializer {
public:
    virtual ~ValueSerializer();

    virtual void writeValue(bool) = 0;
    virtual void writeValue(short) = 0;
    virtual void writeValue(unsigned short) = 0;
    virtual void writeValue(int) = 0;
    virtual void writeValue(unsigned int) = 0;
    virtual void writeValue(long) = 0;
    virtual void writeValue(unsigned long) = 0;
    virtual void writeValue(long long) = 0;
    virtual void writeValue(unsigned long long) = 0;
    virtual void writeValue(float) = 0;
    virtual void writeValue(double) = 0;
    virtual void writeValue(std::string_view) = 0;
    virtual void writeValue(std::nullptr_t) = 0; // write null explicitly
    virtual void writeValue(std::nullopt_t) = 0; // discard current value

    // helper for string literals
    void writeValue(const char* str) { writeValue(std::string_view(str)); }

    virtual std::ostream& openStringStream() = 0;
    virtual void closeStringStream() = 0;

    virtual void pushKey(std::string_view key) = 0;

    virtual void openObject() = 0;
    virtual void closeObject() = 0;
    virtual void openArray() = 0;
    virtual void closeArray() = 0;

    void throwException(const std::string& msg);
};

} // namespace huse
