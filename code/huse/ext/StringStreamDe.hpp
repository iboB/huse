// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "StringStream.hpp"
#include "../DeNode.hpp"
#include <itlib/mem_streambuf.hpp>
#include <istream>

namespace huse {

template <typename De>
class DeStringStream : public DeNode<De> {
public:
    using Node = DeNode<De>;

    DeStringStream(const DeStringStream&) = delete;
    DeStringStream(DeStringStream&&) noexcept = delete;

    template <typename T>
    DeStringStream& operator>>(T& t) {
        m_stream >> t;
        return *this;
    }

    template <typename T>
    DeStringStream& operator&(T& t) {
        m_stream >> t;
        return *this;
    }

    std::istream& get() { return m_stream; }

private:
    explicit DeStringStream(DeNode<De>&& d, std::string_view str) noexcept
        : Node(std::move(d))
        , m_streambuf(str.data(), str.size())
        , m_stream(&m_streambuf)
    {}

    itlib::mem_istreambuf<char> m_streambuf;
    std::istream m_stream;

    template <typename D>
    friend DeStringStream<D> huse_open(DeNode<D> node, StringStream);
};

template <typename D>
DeStringStream<D> huse_open(DeNode<D> node, StringStream) {
    std::string_view str;
    node.val(str);
    return DeStringStream<D>(std::move(node), str);
}

} // namespace huse
