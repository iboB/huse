// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include "Type.hpp"
#include "Ctx.hpp"
#include <pojobuf/value.hpp>
#include <itlib/small_vector.hpp>
#include <limits>

namespace huse {

class Trace;

class HUSE_API PojobufDeState {
public:
    PojobufDeState() noexcept = default;

    // these can tehcnically be implemented, but it's somewhat complex and there is no need at this time
    PojobufDeState(const PojobufDeState&) = delete;
    PojobufDeState& operator=(const PojobufDeState&) = delete;

    PojobufDeState(PojobufDeState&&) noexcept = default;
    PojobufDeState& operator=(PojobufDeState&&) noexcept = default;

    virtual ~PojobufDeState();

    using ValueType = pojobuf::value;

    static Type convertType(pojobuf::value_type pt) {
        using enum pojobuf::value_tag;
        switch (*pt) {
        case undefined: return Type::Undefined;
        case null: return Type::Null;
        case true_: [[fallthrough]];
        case false_: return Type::Boolean;
        case int32: [[fallthrough]];
        case int64: return Type::Integer;
        case real: return Type::Float;
        case blob: [[fallthrough]];
        case string: return Type::String;
        case array: return Type::Array;
        case object: [[fallthrough]];
        case sorted_object: return Type::Object;
        case custom: [[fallthrough]];
        default: return Type::Custom;
        }
    }

    struct StackEntry {
        pojobuf::value value;
        uint32_t curIndex = 0;
    };

    const StackEntry& top() const noexcept {
        return m_stack.back();
    }
    StackEntry& top() noexcept {
        return m_stack.back();
    }

    uintptr_t topId() const {
        return reinterpret_cast<uintptr_t>(top().value.data_ptr());
    }

    Trace getTrace() const;

    [[noreturn]] void throwException(std::string msg) const;

    void push(const pojobuf::value& val) {
        m_stack.push_back({val});
    }
    void pop() noexcept {
        assert(!m_stack.empty());
        m_stack.pop_back();
    }

    Type topType() const noexcept {
        return convertType(top().value.type());
    }

    bool topIsUndefined() const noexcept {
        return top().value.type().is_undefined();
    }

    void ensureTopIsArray() const {
        if (!top().value.type().is_array()) [[unlikely]] throwException("not an array");
    }
    uint32_t topArraySize() const noexcept {
        return uint32_t(top().value.compound_length());
    }
    bool topArrayDone() const noexcept {
        auto& top = this->top();
        return top.curIndex >= top.value.compound_length();
    }
    void topArrayResetIndex() noexcept {
        this->top().curIndex = 0;
    }
    void topArrayPushElement(uint32_t index) noexcept {
        auto& top = this->top();
        top.curIndex = index + 1;
        push(top.value.array_element_at_safe(index));
    }
    void topArrayPushNextElement() noexcept {
        topArrayPushElement(top().curIndex);
    }

    void ensureTopIsObject() const {
        if (!top().value.type().is_object()) [[unlikely]] throwException("not an object");
    }
    uint32_t topObjectSize() const noexcept {
        return uint32_t(top().value.compound_length());
    }
    bool topObjectDone() const noexcept {
        auto& top = this->top();
        return top.curIndex >= top.value.compound_length();
    }
    void topObjectResetIteration() noexcept {
        this->top().curIndex = 0;
    }
    void topObjectPushKey(std::string_view key) noexcept {
        auto& top = this->top();
        auto& value = top.value;

        // optimistically check for the next key first, as it's likely to be the one we want
        auto i = top.curIndex++;
        if (i < value.compound_length()) {
            auto k = value.object_key_at(i);
            if (k == key) {
                push(value.object_value_at(i));
                return;
            }
        }

        // have to search
        i = uint32_t(value.find_object_key(key));
        top.curIndex = i + 1;

        if (i < value.compound_length()) {
            m_lastMissingKey = {};
            push(value.object_value_at(i));
        }
        else {
            m_lastMissingKey = key;
            push(pojobuf::value{}); // push undefined
        }
    }
    std::string_view topObjectPushNextKey() noexcept {
        auto& top = this->top();
        auto i = top.curIndex++;
        auto [k, v] = top.value.object_element_at_safe(i);
        m_lastMissingKey = {};
        push(v);
        return k;
    }

    Ctx ctx;

protected:
    itlib::small_vector<StackEntry, 32> m_stack;
    std::string_view m_lastMissingKey; // trace for lazy exceptions
};

inline void huseState_serde(const PojobufDeState& self, bool& out) {
    auto t = self.top().value.type();
    if (t.is_true()) {
        out = true;
    }
    else if (t.is_false()) {
        out = false;
    }
    else {
        self.throwException("not a boolean");
    }
}

template <std::integral I>
inline void huseState_serde(const PojobufDeState& self, I& out) {
    auto& value = self.top().value;
    if (!value.type().is_integer()) [[unlikely]] self.throwException("not an integer");

    auto ret = value.integer_value();

    if constexpr (std::is_unsigned_v<I>) {
        if (ret < 0) self.throwException("negative integer");
    }
    if constexpr (sizeof(I) < sizeof(ret)) {
        if (ret < std::numeric_limits<I>::min() || ret > std::numeric_limits<I>::max()) {
            self.throwException("integer out of range");
        }
    }

    out = I(ret);
}

template <std::floating_point F>
inline void huseState_serde(const PojobufDeState& self, F& out) {
    auto& value = self.top().value;
    if (value.type().is_number()) {
        out = F(value.real_value_safe());
    }
    else {
        self.throwException("not a number");
    }
}

inline void huseState_serde(const PojobufDeState& self, std::string_view& out) {
    auto& value = self.top().value;
    if (!value.type().is_string()) self.throwException("not a string");
    out = value.string_value();
}

inline void huseState_serde(const PojobufDeState& self, std::string& out) {
    std::string_view sv;
    huseState_serde(self, sv);
    out.assign(sv);
}

inline void huseState_serde(const PojobufDeState& self, std::nullptr_t) {
    auto t = self.top().value.type();
    if (!t.is_null()) self.throwException("not null");
}

} // namespace huse
