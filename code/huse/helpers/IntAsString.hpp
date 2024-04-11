// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../Serializer.hpp"
#include "../Deserializer.hpp"

#include "../impl/Charconv.hpp"

namespace huse {
namespace impl {
template <typename T>
bool int_from_string(std::string_view str, T& out, int base = 10) {
    auto end = str.data() + str.length();
    auto ret = HUSE_CHARCONV_NAMESPACE::from_chars(str.data(), end, out, base);
    if (ret.ec != std::errc{}) return false;
    return ret.ptr == end;
}
}

// serialize integers as strings

// with optional fallback but int has to be a template argument of struct
template <typename Int>
struct IntAsStringOpt {
    std::optional<Int> emptyStringVal;
    explicit IntAsStringOpt(std::optional<Int> esv = std::nullopt) : emptyStringVal(esv) {}

    void operator()(SerializerNode& n, Int i) const {
        char buf[21] = {0};
        auto res = HUSE_CHARCONV_NAMESPACE::to_chars(buf, buf + sizeof(buf), i);
        n.val(std::string_view(buf, res.ptr - buf));
    }

    void operator()(DeserializerNode& n, Int& i) const {
        std::string_view val;
        n.val(val);
        if (val.empty() && emptyStringVal) {
            i = *emptyStringVal;
        }
        else if (!impl::int_from_string(val, i)) {
            throwDeserializerException(n._s(), "not an integer");
        }
    }
};

// ... or without optional fallback and no template arg
struct IntAsString {
    template <typename N, typename V>
    void operator()(N& n, V& v) const {
        n.cval(v, IntAsStringOpt<V>{});
    }
};

}
