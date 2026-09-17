// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "DeNode.hpp"

namespace huse {

template <typename De>
class DeRoot final : public De, public DeNode<De> {
    // this was converted to static function from a class method
    // because the method led to gcc (erroneously) creating a vtable for DeRoot
    // which leads to ubsan errors
    // the static function works around the issue
    static De& initRoot(De& d) {
        d.initRoot();
        return d;
    }
public:
    template <typename... Args>
    DeRoot(Args&&... args)
        : De(std::forward<Args>(args)...)
        , DeNode<De>(initRoot(*this), true)
    {}

    // copy and move can be implemented (maintaining self-reference), but it's not needed at this time
    DeRoot(const DeRoot&) = delete;
    DeRoot& operator=(const DeRoot&) = delete;
    DeRoot(DeRoot&&) = delete;
    DeRoot& operator=(DeRoot&&) = delete;
};

} // namespace huse
