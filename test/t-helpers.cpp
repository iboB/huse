// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <huse/json/DeserializerRoot.hpp>
#include <huse/json/SerializerRoot.hpp>

#include <huse/helpers/Identity.hpp>
#include <huse/helpers/StdVector.hpp>
#include <huse/helpers/StdMap.hpp>
#include <huse/helpers/IntAsString.hpp>

#include <huse/Exception.hpp>

#include <doctest/doctest.h>

#include <sstream>

TEST_SUITE_BEGIN("huse");

template <typename T, typename C>
struct ObjWrap {
    T t;
    C c;

    ObjWrap(T t, C c) : t(t), c(c) {}
    ObjWrap(C c) : c(c) {}

    template <typename Node, typename Self>
    static void sh(Node& n, Self& s) {
        auto obj = n.obj();
        obj.cval("t", s.t, s.c);
    }
    template <typename S>
    void huseSerialize(huse::SerializerNode<S>& n) const { sh(n, *this); }
    template <typename D>
    void huseDeserialize(huse::DeserializerNode<D>& n) { sh(n, *this); }
};

template <typename T, typename C>
T cclone(const T& val, C c, std::optional<std::string> check = std::nullopt) {
    ObjWrap<T, C> wrap = {val, c};

    std::stringstream sout;
    {
        huse::json::SerializerRoot s(sout, !check.has_value());
        s.val(wrap);
    }

    auto json = sout.str();
    if (check) {
        CHECK(json == *check);
    }

    ObjWrap<T, C> ret(c);
    huse::json::DeserializerRoot d(json.data(), json.length());
    d.val(ret);
    return ret.t;
}

template <typename T>
T sclone(const T& val, std::optional<std::string> check = std::nullopt) {
    return cclone(val, huse::Identity{}, check);
}

TEST_CASE("vec") {
    std::vector<int> ints = {1, 2, 3, 4};
    CHECK(ints == sclone(ints));

    std::vector<std::string> strings = {"foo", "baz", "something long trololo and more and more", "42"};
    CHECK(strings == sclone(strings));
}

TEST_CASE("map") {
    std::map<std::string, int> si = {{"a", 1}, {"foo", 43}, {"bagavag", 32}};
    auto siclone = sclone(si, R"({"t":{"a":1,"bagavag":32,"foo":43}})");
    CHECK(siclone == si);

    std::map<int, std::string> is = {{1, "foo"}, {4, "dsfsd"}, {-5, "boo"}};
    auto isclone = sclone(is);
    CHECK(isclone == is);
}

TEST_CASE("istr") {
    const int i = 551122;
    const auto ic = cclone(i, huse::IntAsString{}, R"({"t":"551122"})");
    CHECK(ic == i);

    const int64_t u64 = std::numeric_limits<int64_t>::min();
    const auto u64c = cclone(u64, huse::IntAsString{});
    CHECK(u64c == u64);

    const std::string json = R"({"t":""})";
    huse::json::DeserializerRoot d(json);

    {
        ObjWrap<int, huse::IntAsString> w({});
        CHECK_THROWS_WITH_AS(d.val(w), R"(not an integer)", huse::DeserializerException);
    }

    {
        ObjWrap<int, huse::IntAsStringOpt<int>> w(huse::IntAsStringOpt<int>{73});
        d.val(w);
        CHECK(w.t == 73);
    }
}

