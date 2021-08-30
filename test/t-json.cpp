// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <doctest/doctest.h>

#include <huse/JsonDeserializer.hpp>
#include <huse/JsonSerializer.hpp>

#include <sstream>

TEST_SUITE_BEGIN("json");

struct JsonSerializerPack
{
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

struct JsonSerializeTester
{
    std::optional<JsonSerializerPack> pack;

    huse::JsonSerializer& make(bool pretty = false)
    {
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
    JsonSerializeTester j;

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
        obj.val("bool2",false);
        obj.val("float",3.1f);
        obj.val("int", yup);
        obj.val("nope", nope);
        obj.val("unsigned-long-long",900000000000000ULL);
        obj.val("str", "b\n\\g\t\033sdf");
    }
    CHECK(j.str() == R"({"array":[1,2,3,4],"bool":true,"bool2":false,"float":3.1,"int":-3,"unsigned-long-long":900000000000000,"str":"b\n\\g\t\u001bsdf"})");

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

huse::JsonDeserializer makeD(std::string_view str)
{
    return huse::JsonDeserializer::fromConstString(str);
}

TEST_CASE("simple deserialize")
{
    {
        auto d = makeD("[]");
        auto ar = d.ar();
        CHECK(ar.length() == 0);
    }

    {
        auto d = makeD(R"({"array":[1,2,3,4],"bool":true,"bool2":false,"float":3.1,"int":-3,"unsigned-long-long":900000000000000,"str":"b\n\\g\t\u001bsdf"})");
        CHECK(d.type().is(huse::Deserializer::Type::Object));
        auto obj = d.obj();
        CHECK(obj.type().is(huse::Deserializer::Type::Object));
        {
            auto ar = obj.ar("array");
            CHECK(ar.type().is(huse::Deserializer::Type::Array));
            CHECK(ar.length() == 4);
            int i;
            ar.index(2).val(i);
            CHECK(i == 3);
            double d;
            ar.index(0).val(d);
            CHECK(d == 1.0);
            unsigned ii;
            ar.val(ii);
            CHECK(ii == 2);
        }
        bool b;
        obj.val("bool", b);
        CHECK(b);

        auto q = obj.nextkey();
        CHECK(!!q);
        CHECK(q.name == "bool2");
        CHECK(q.node->type().is(huse::Deserializer::Type::Boolean));
        CHECK(q.node->type().is(huse::Deserializer::Type::False));
        q->val(b);
        CHECK(!b);

        std::string str;
        obj.val("str", str);
        CHECK(str == "b\n\\g\t\033sdf");

        CHECK(!obj.nextkey());

        float f;
        obj.val("float", f);
        CHECK(f == 3.1f);

        int i;
        obj.val("int", i);
        CHECK(i == -3);

        unsigned long long ull;
        obj.val("unsigned-long-long", ull);
        CHECK(ull == 900000000000000);

        auto& inode = obj.key("int");
        CHECK(inode.type().is(huse::Deserializer::Type::Integer));
        CHECK(inode.type().is(huse::Deserializer::Type::Number));
        CHECK(!inode.type().is(huse::Deserializer::Type::Float));

        auto& fnode = obj.key("float");
        CHECK(fnode.type().is(huse::Deserializer::Type::Float));
        CHECK(fnode.type().is(huse::Deserializer::Type::Number));

        CHECK(obj.key("str").type().is(huse::Deserializer::Type::String));

        CHECK(obj.key("array").type().is(huse::Deserializer::Type::Array));
    }
}
