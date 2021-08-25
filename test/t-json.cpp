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

struct JsonOut {
    std::ostringstream sout;
    huse::JsonSerializer s;

    JsonOut(bool pretty = false)
        : s(sout, pretty)
    {}

    huse::JsonSerializer* operator->() {
        return &s;
    }

    std::string str() {
        return sout.str();
    }
};

TEST_CASE("simple serialize")
{
    {
        JsonOut j;
        j->val(5);
        CHECK(j.str() == "5");
    }
    {
        JsonOut j;
        j->val(nullptr);
        CHECK(j.str() == "null");
    }
    {
        JsonOut j;
        {
            auto obj = j->obj();
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
    }
}
