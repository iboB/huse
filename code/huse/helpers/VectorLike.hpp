// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../SerializerNode.hpp"
#include "../DeserializerNode.hpp"

namespace huse {

// a serialization functor for vector-like objects
struct VectorLike{
    template <typename S, typename Vec>
    void operator()(SerializerNode<S>& n, const Vec& vec) const  {
        auto ar = n.ar();
        for (auto& val : vec)
        {
            ar.val(val);
        }
    }

    template <typename Vec>
    void operator()(DeserializerNode& n, Vec& vec) const {
        auto ar = n.ar();
        auto len = ar.size();
        vec.resize(size_t(len));
        for (auto& val : vec)
        {
            ar.val(val);
        }
    }
};

}
