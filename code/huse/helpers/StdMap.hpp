// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "MapLike.hpp"

#include <map>

namespace huse {
template <typename S, typename K, typename V, typename C, typename A>
void huseSerialize(SerializerNode<S>& n, const std::map<K, V, C, A>& map) {
    MapLike{}(n, map);
}
template <typename K, typename V, typename C, typename A>
void huseDeserialize(DeserializerNode& n, std::map<K, V, C, A>& map) {
    MapLike{}(n, map);
}
}
