// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "SerNode.hpp"

namespace huse {

template <typename Ser>
class SerRoot final : public Ser, public SerNode<Ser> {
public:
    template <typename... Args>
    SerRoot(Args&&... args)
        : Ser(std::forward<Args>(args)...)
        , SerNode<Ser>((Ser&)*this)
    {}

    // copy and move can be implemented (maintaining self-reference), but it's not needed at this time
    SerRoot(const SerRoot&) = delete;
    SerRoot& operator=(const SerRoot&) = delete;
    SerRoot(SerRoot&&) = delete;
    SerRoot& operator=(SerRoot&&) = delete;
};

} // namespace huse
