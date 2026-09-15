// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../SerNode.hpp"
#include "../DeNode.hpp"
#include <optional>

namespace huse {

template <typename S, typename T>
void huse_serde(const SerNode<S>& n, const std::optional<T>& opt) {
    if (!opt) {
        n.discard();
    }
    else {
        n.val(*opt);
    }
}

template <typename D, typename T>
void huse_serde(const DeNode<D>& n, std::optional<T>& opt) {
    if (!n) {
        opt = {};
    }
    else {
        n.val(opt.emplace());
    }
}

} // namespace huse
