// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "../Serializer.hpp"
#include "../Deserializer.hpp"

#include <vector>

namespace huse
{

template <typename T, typename A>
void huseSerialize(SerializerNode& n, const std::vector<T, A>& vec)
{
    auto ar = n.ar();
    for (auto& val : vec)
    {
        ar.val(val);
    }
}

template <typename T, typename A>
void huseDeserialize(DeserializerNode& n, std::vector<T, A>& vec)
{
    auto ar = n.ar();
    auto len = ar.length();
    vec.resize(size_t(len));
    for (auto& val : vec)
    {
        ar.val(val);
    }
}

}
