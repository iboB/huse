// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <huse/json/DeRoot.hpp>
#include <huse/json/SerRoot.hpp>
#include <sstream>

struct JsonSerPack {
    std::ostringstream sout;
    std::optional<huse::json::SerState> s;

    JsonSerPack(bool pretty = false) {
        s.emplace(sout, pretty);
    }

    huse::SerNode<huse::json::SerState> node() {
        return huse::SerNode(*s);
    }

    std::string str() {
        s.reset();
        return sout.str();
    }
};

struct JsonSerTester {
    std::optional<JsonSerPack> pack;

    huse::SerNode<huse::json::SerState> make(bool pretty) {
        assert(!pack);
        pack.emplace(pretty);
        return pack->node();
    }

    huse::SerNode<huse::json::SerState> compact() { return make(false); }
    huse::SerNode<huse::json::SerState> pretty() { return make(true); }

    std::string str() {
        std::string ret = pack->str();
        pack.reset();
        return ret;
    }
};
