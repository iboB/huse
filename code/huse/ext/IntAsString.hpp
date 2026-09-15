// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <pojobuf/bits/charconv.hpp>

namespace huse {

struct IntAsString {
    template <typename Ser, typename I>
    void operator()(const SerNode<Ser>& n, const I& i) const {
        char buf[21];
        auto res = POJOBUF_CHARCONV_NAMESPACE::to_chars(buf, buf + sizeof(buf), i);
        n.val(std::string_view(buf, res.ptr - buf));
    }

    template <typename De, typename I>
    void operator()(const DeNode<De>& n, I& i) const {
        std::string_view str;
        n.val(str);
        auto end = str.data() + str.length();
        auto ret = POJOBUF_CHARCONV_NAMESPACE::from_chars(str.data(), end, i);
        if (ret.ec != std::errc{} || ret.ptr != end) [[unlikely]] {
            n.throwException("not an integer-string");
        }
    }
};

} // namespace huse
