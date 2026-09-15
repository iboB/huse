// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../api.h"
#include "../ISerState.hpp"
#include <pojobuf/ostream_string_sink.hpp>
#include <iosfwd>
#include <memory>
#include <optional>

namespace huse::json {

class HUSE_API SerState : public ISerState {
public:
    SerState(std::ostream& out, bool pretty = false);
    ~SerState();

    virtual void writeValue(bool val) final override;
    virtual void writeValue(short val) final override;
    virtual void writeValue(unsigned short val) final override;
    virtual void writeValue(int val) final override;
    virtual void writeValue(unsigned int val) final override;
    virtual void writeValue(long val) final override;
    virtual void writeValue(unsigned long val) final override;
    virtual void writeValue(long long val) final override;
    virtual void writeValue(unsigned long long val) final override;
    virtual void writeValue(float val) final override;
    virtual void writeValue(double val) final override;
    virtual void writeValue(std::string_view val) final override;
    virtual void writeValue(std::nullptr_t) final override;

    virtual void discardTop() noexcept final override;

    virtual void topObjectPushKey(std::string_view key) final override;

    // special writers
    void writeRawJsonValue(std::string_view json);

    std::ostream& out() { return m_sink.stream; }

    virtual void pushArrayValue() final override;
    virtual void topArrayClose() final override;
    virtual void pushObjectValue() final override;
    virtual void topObjectClose() final override;
    virtual std::ostream& pushStringStream() final override;
    virtual void topStringStreamClose() final override;

    virtual uint32_t curStackDepth() const noexcept final override;
    virtual void setRenderCompact() noexcept final override;
    virtual bool topIsCompact() const noexcept final override;

    // logically private
    static constexpr size_t ImplBuf_Size = 64;
    struct Impl;
private:
    template <typename T> void writeNumber(T num);

    pojobuf::ostream_string_sink<> m_sink;

    alignas (std::max_align_t) std::byte m_implBuf[ImplBuf_Size];
    Impl* m_impl;

    struct JsonOStream;
    std::unique_ptr<std::optional<JsonOStream>> m_stringStream;

};

struct RawJsonValue {
    std::string_view str;
};
inline void huseState_serde(SerState& self, const RawJsonValue& val) {
    self.writeRawJsonValue(val.str);
}

} // namespace huse::json
