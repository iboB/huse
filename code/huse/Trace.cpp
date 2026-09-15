// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Trace.hpp"
#include "Type.toString.hpp"

namespace huse {

std::string Trace::describe() const {
    std::string str = "root";

    for (auto& e : entries) {
        if (e.key) {
            str += ".\"";
            str += *e.key;
            str += '"';
        }
        else {
            str += "[";
            str += std::to_string(e.index);
            str += "]";
        }
    }

    str += '<';
    str += Type_toString(topType);
    str += '>';

    return str;
}

} // namespace huse
