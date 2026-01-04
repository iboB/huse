// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "OpenTags.hpp"
#include "impl/Assert.hpp"
#include <splat/warnings.h>
#include <string_view>
#include <string>
#include <cstddef>
#include <optional>

// Yes, yes, we're propagating the warning disable to all includers
// but this class is supposed to be inherited virtually and this triggers the
// stupid and absoltely pointless MSVC warning.
// MSVC, how are we supposed to use virtual inheritance if not like this?
DISABLE_MSVC_WARNING(4250)

namespace huse {
class CtxObj;

class HUSE_API Serializer {
public:
    virtual ~Serializer();

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

    int open(Object) { openObject(); return 0; }
    int open(Array) { openArray(); return 0; }
    std::ostream& open(StringStream) { return openStringStream(); }

    void throwException(const std::string& msg);

    // stack control (for debugging purposes)
    int curNodeId() const noexcept {
        return m_curNodeId;
    }
    int getNewNodeId() noexcept {
        m_curNodeId = m_freeNodeId++;
        return m_curNodeId;
    }
    void releaseNodeId([[maybe_unused]] int released, int newCur) noexcept {
        HUSE_ASSERT_USAGE(released == m_curNodeId, "node id mismatch");
        m_curNodeId = newCur;
    }
private:
    int m_freeNodeId = 0;
    int m_curNodeId = -1;
};

} // namespace huse
