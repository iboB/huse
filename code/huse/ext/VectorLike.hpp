// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Identity.hpp"
#include "../SerNode.hpp"
#include "../DeNode.hpp"

namespace huse {

// a serialization functor for vector-like objects
template <typename SerdeValue = Identity>
struct VectorLike {
    SerdeValue serdeValue;

    VectorLike() = default;
    explicit VectorLike(SerdeValue sdv)
        : serdeValue(std::move(sdv))
    {}

    template <typename S, typename Vec>
    void operator()(const SerNode<S>& n, const Vec& vec) const {
        auto ar = n.ar();
        for (auto& val : vec) {
            ar.cval(val, serdeValue);
        }
    }

    template <typename D, typename Vec>
    void operator()(const DeNode<D>& n, Vec& vec) const {
        auto ar = n.ar();
        auto len = ar.size();
        vec.resize(size_t(len));
        for (auto& val : vec) {
            ar.cval(val, serdeValue);
        }
    }
};

} // namespace huse
