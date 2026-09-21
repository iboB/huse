// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "Ctx.hpp"
#include <string_view>
#include <string>
#include <cstdint>
#include <cstddef>
#include <concepts>

namespace huse {

class HUSE_API ISerState {
public:
    virtual ~ISerState();

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

    virtual void discardTop() noexcept = 0; // discard current value

    virtual uint32_t curStackDepth() const noexcept = 0;
    virtual void setRenderCompact() noexcept = 0;
    virtual bool topIsCompact() const noexcept = 0;

    virtual void pushArrayValue() = 0;
    virtual void topArrayClose() = 0;

    virtual void pushObjectValue() = 0;
    virtual void topObjectPushKey(std::string_view key) = 0;
    virtual void topObjectClose() = 0;

    virtual std::ostream& pushStringStream() = 0;
    virtual void topStringStreamClose() = 0;

    [[noreturn]] virtual void throwException(std::string msg) const;

    Ctx ctx;
};

inline void huseState_serde(ISerState& self, bool val) {
    self.writeValue(val);
}

template <std::integral I>
inline void huseState_serde(ISerState& self, I val) {
    self.writeValue(val);
}

template <std::floating_point F>
inline void huseState_serde(ISerState& self, F val) {
    self.writeValue(val);
}

inline void huseState_serde(ISerState& self, std::string_view val) {
    self.writeValue(val);
}

inline void huseState_serde(ISerState& self, std::nullptr_t) {
    self.writeValue(std::nullptr_t{});
}

// helper for literals
inline void huseState_serde(ISerState& self, const char* val) {
    self.writeValue(std::string_view(val));
}

} // namespace huse
