// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "DeNode.hpp"

namespace huse {

template <typename De>
class DeRoot : public De, public DeNode<De> {
    De& initRoot() {
        De::initRoot();
        return *this;
    }
public:
    template <typename... Args>
    DeRoot(Args&&... args)
        : De(std::forward<Args>(args)...)
        , DeNode<De>(initRoot(), true)
    {}

    // copy and move can be implemented (maintaining self-reference), but it's not needed at this time
    DeRoot(const DeRoot&) = delete;
    DeRoot& operator=(const DeRoot&) = delete;
    DeRoot(DeRoot&&) = delete;
    DeRoot& operator=(DeRoot&&) = delete;
};

} // namespace huse
