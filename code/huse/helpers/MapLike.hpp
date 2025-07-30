// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../Serializer.hpp"
#include "../Deserializer.hpp"

namespace huse {

// a serialization functor for map-like objects
struct MapLike {
    template <typename Map>
    void operator()(SerializerNode& n, const Map& map) const {
        if constexpr (std::is_convertible_v<typename Map::key_type, std::string_view>) {
            auto obj = n.obj();
            for (auto& val : map) {
                obj.val(val.first, val.second);
            }
        }
        else {
            auto ar = n.ar();
            for (auto& val : map) {
                auto pair = ar.obj();
                pair.val("key", val.first);
                pair.val("value", val.second);
            }
        }
    }

    template <typename Map>
    void operator()(DeserializerNode& n, Map& map) const {
        using KvPair = std::pair<typename Map::key_type, typename Map::mapped_type>;

        if constexpr (std::is_convertible_v<typename Map::key_type, std::string_view>) {
            auto obj = n.obj();
            const int len = obj.length();
            for (int i = 0; i < len; ++i) {
                KvPair val;
                obj.keyval(val.first, val.second);
                map.emplace(std::move(val));
            }
        }
        else {
            auto ar = n.ar();
            const int len = ar.length();
            for (int i = 0; i < len; ++i) {
                KvPair val;
                auto pair = ar.obj();
                pair.val("key", val.first);
                pair.val("value", val.second);
                map.emplace(std::move(val));
            }
        }
    }
};

}
