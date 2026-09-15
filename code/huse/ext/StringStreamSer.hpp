// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "StringStream.hpp"
#include "../SerNode.hpp"
#include <ostream>

namespace huse {

template <typename Ser>
class SerStringStream : public SerCompound<Ser> {
    std::ostream& m_stream;
public:
    using Node = SerNode<Ser>;
    using Super = SerCompound<Ser>;

    ~SerStringStream() {
        this->assertIsTop();
        if (this->m_ownsClose) {
            this->m_state.topStringStreamClose();
        }
    }

    SerStringStream(const SerStringStream&) noexcept = default;
    template <std::derived_from<Ser> Ser2>
    SerStringStream(const SerStringStream<Ser2>& other) noexcept
        : Super(other)
    {}

    SerStringStream(SerStringStream&&) noexcept = default;
    template <std::derived_from<Ser> Ser2>
    SerStringStream(SerStringStream<Ser2>&& other) noexcept
        : Super(std::move(other))
    {}

    using Super::_state;

    template <typename T>
    SerStringStream& operator<<(const T& t) {
        m_stream << t;
        return *this;
    }

    template <typename T>
    SerStringStream& operator&(const T& t) {
        m_stream << t;
        return *this;
    }

    std::ostream& get() { return m_stream; }

    template <typename S>
    friend SerStringStream<S> huse_open(const SerNode<S>& node, StringStream);

private:
    explicit SerStringStream(Ser& state, std::ostream& stream)
        : Super(state, true)
        , m_stream(stream)
    {}
};

template <typename S>
SerStringStream<S> huse_open(const SerNode<S>& node, StringStream) {
    auto& state = node._state();
    return SerStringStream<S>(state, state.pushStringStream());
}

} // namespace huse
