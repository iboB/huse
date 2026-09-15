// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "JsonTestUtil.hpp"
#include <huse/json/DeRoot.hpp>
#include <huse/json/SerRoot.hpp>
#include <huse/ext/StringStreamSer.hpp>
#include <huse/ext/StringStreamDe.hpp>

#include <doctest/doctest.h>

TEST_CASE("ser stream") {
    JsonSerTester j;

    j.compact().open(huse::StringStream{}) << "xx " << 123;
    CHECK(j.str() == R"("xx 123")");

    {
        auto root = j.compact();
        auto s = root.open(huse::StringStream{});
        s << -5;
        s.get().put(' ');
        s.get().write("abc", 3);
    }
    CHECK(j.str() == R"("-5 abc")");

    {
        auto root = j.compact();
        auto s = root.open(huse::StringStream{});
        s << "b\n\\g";
        s.get().put('\t');
        s.get().put(27);
        s << "sdf";
    }
    CHECK(j.str() == R"("b\n\\g\t\u001bsdf")");
}

TEST_CASE("de stream")
{
    huse::json::DeRoot d(huse::Parse, R"({"string":"aa bbb c"})");
    auto o = d.obj();

    {
        std::string a, b, c;
        o.key("string").open(huse::StringStream{}) >> a >> b >> c;
        CHECK(a == "aa");
        CHECK(b == "bbb");
        CHECK(c == "c");
    }

    {
        std::string a, b, c;
        auto s = o.key("string").open(huse::StringStream{});
        s >> a >> b >> c;
        CHECK(a == "aa");
        CHECK(b == "bbb");
        CHECK(c == "c");
        CHECK(s.get().eof());
    }
}

struct vector2 { int x, y; };
std::ostream& operator<<(std::ostream& o, const vector2& v)
{
    o << '(' << v.x << ';' << v.y << ')';
    return o;
}

std::istream& operator>>(std::istream& i, vector2& v)
{
    i.get(); // (
    i >> v.x;
    i.get(); // ;
    i >> v.y;
    i.get(); // )
    return i;
}

struct MultipleValuesAsString {
    std::string a;
    vector2 b;

    template <typename N, typename MVS>
    static void huse_serde(N& n, MVS& self) {
        n.obj().key("data").open(huse::StringStream{}) & self.b & self.a;
    }
};

TEST_CASE("stream roundtrip") {
    MultipleValuesAsString mvs = {"xyz", {34, 88}};
    JsonSerTester j;
    j.compact().val(mvs);

    auto json = j.str();

    MultipleValuesAsString cc;
    {
        huse::json::DeRoot d(huse::Parse, json);
        d.val(cc);
    }

    CHECK(mvs.a == cc.a);
    CHECK(mvs.b.x == cc.b.x);
    CHECK(mvs.b.y == cc.b.y);
}
