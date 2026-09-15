// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "VectorLike.hpp"

#include <vector>

namespace huse {

template <typename S, typename T, typename A>
void huse_serde(const SerNode<S>& n, const std::vector<T, A>& vec) {
    VectorLike{}(n, vec);
}

template <typename D, typename T, typename A>
void huse_serde(const DeNode<D>& n, std::vector<T, A>& vec) {
    VectorLike{}(n, vec);
}

} // namespace huse
