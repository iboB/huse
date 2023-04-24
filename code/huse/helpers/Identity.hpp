// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace huse {

// an identity function for cval useful in a template context
struct Identity {
    template <typename Node, typename Val>
    void operator()(Node& node, Val& val) const {
        node.val(val);
    }
};

}
