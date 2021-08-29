// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <doctest/doctest.h>

#include <huse/JsonSerializer.hpp>

#include <sstream>

TEST_SUITE_BEGIN("json");

struct JsonSerializerPack {
    std::ostringstream sout;
    std::optional<huse::JsonSerializer> s;

    JsonSerializerPack(bool pretty)
    {
        s.emplace(sout, pretty);
    }

    std::string str()
    {
        s.reset();
        return sout.str();
    }
};

struct JsonOutTester {
    std::optional<JsonSerializerPack> pack;

    huse::JsonSerializer& make(bool pretty = false) {
        assert(!pack);
        pack.emplace(pretty);
        return *pack->s;
    }

    huse::JsonSerializer& compact() { return make(false); }
    huse::JsonSerializer& pretty() { return make(true); }

    std::string str()
    {
        std::string ret = pack->str();
        pack.reset();
        return ret;
    }
};

TEST_CASE("simple serialize")
{
    JsonOutTester j;

    j.compact().val(5);
    CHECK(j.str() == "5");

    j.compact().val(nullptr);
    CHECK(j.str() == "null");

    {
        auto obj = j.compact().obj();
        {
            auto ar = obj.ar("array");
            for (int i = 1; i < 5; ++i) ar.val(i);
        }
        std::optional<int> nope;
        std::optional<int> yup = -3;
        obj.val("bool",true);
        obj.val("bool",false);
        obj.val("float",3.1f);
        obj.val("int", yup);
        obj.val("nope", nope);
        obj.val("unsigned-long-long",900000000000000ULL);
        obj.val("str", "b\n\\g\t\033sdf");
    }
    CHECK(j.str() == R"({"array":[1,2,3,4],"bool":true,"bool":false,"float":3.1,"int":-3,"unsigned-long-long":900000000000000,"str":"b\n\\g\t\u001bsdf"})");

    j.compact().obj().obj("i1").obj("i2").obj("i3").val("deep", true);
    CHECK(j.str() == R"({"i1":{"i2":{"i3":{"deep":true}}}})");

    {
        auto obj = j.pretty().obj();
        {
            obj.val("pretty", true);
            auto ar = obj.ar("how_much");
            ar.val("very");
            ar.val("very");
            ar.val("much");
        }
    }
    CHECK(j.str() ==
R"({
  "pretty":true,
  "how_much":[
    "very",
    "very",
    "much"
  ]
})"
    );
}
