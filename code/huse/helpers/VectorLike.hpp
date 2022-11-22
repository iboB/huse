// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../Serializer.hpp"
#include "../Deserializer.hpp"

namespace huse
{

// a serialization functor for vector-like objects
struct VectorLike
{
    template <typename Vec>
    void operator()(SerializerNode& n, const Vec& vec)
    {
        auto ar = n.ar();
        for (auto& val : vec)
        {
            ar.val(val);
        }
    }

    template <typename Vec>
    void operator()(DeserializerNode& n, Vec& vec)
    {
        auto ar = n.ar();
        auto len = ar.length();
        vec.resize(size_t(len));
        for (auto& val : vec)
        {
            ar.val(val);
        }
    }
};

}
