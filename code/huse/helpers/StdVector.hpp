// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "VectorLike.hpp"

#include <vector>

namespace huse
{

template <typename S, typename T, typename A>
void huseSerialize(SerializerNode<S>& n, const std::vector<T, A>& vec)
{
    VectorLike{}(n, vec);
}

template <typename T, typename A>
void huseDeserialize(DeserializerNode& n, std::vector<T, A>& vec)
{
    VectorLike{}(n, vec);
}

}
