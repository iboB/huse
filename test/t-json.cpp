// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "JsonTestUtil.hpp"

#include <huse/json/DeRoot.hpp>
#include <huse/json/SerRoot.hpp>
#include <huse/json/Limits.hpp>
#include <huse/Exception.hpp>
#include <huse/ext/ValStdVector.hpp>

#include <doctest/doctest.h>

#include <limits>
#include <cstring>

TEST_CASE("ser simple") {
    JsonSerTester j;

    j.compact().val(5);
    CHECK(j.str() == "5");

    j.compact().val(nullptr);
    CHECK(j.str() == "null");

    {
        auto root = j.compact();
        auto obj = root.obj();

        CHECK(&obj._state() == &(*j.pack->s));

        {
            auto ar = obj.ar("array");
            for (int i = 1; i < 5; ++i) ar.val(i);
            ar.val(huse::json::RawJsonValue{"42"});
        }
        obj.val("bool",true);
        obj.val("bool2",false);
        obj.val("float",3.1f);
        CHECK(obj.optval("int", -3));
        obj.val("unsigned-long-long",900000000000000ULL);
        obj.val("str", "b\n\\g\t\033sdf");
    }
    CHECK(j.str() == R"({"array":[1,2,3,4,42],"bool":true,"bool2":false,"float":3.1,"int":-3,"unsigned-long-long":900000000000000,"str":"b\n\\g\t\u001bsdf"})");

    j.compact().obj().obj("i1").obj("i2").obj("i3").val("deep", true);
    CHECK(j.str() == R"({"i1":{"i2":{"i3":{"deep":true}}}})");

    {
        auto root = j.pretty();
        auto obj = root.obj();
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

TEST_CASE("ser exceptions") {
    {
        CHECK_THROWS_WITH_AS(
            JsonSerPack().node().val(1ull << 55),
            "Invalid JSON number value",
            huse::SerException
        );
    }

    {
        CHECK_THROWS_WITH_AS(
            JsonSerPack().node().val(-(1ll << 55)),
            "Invalid JSON number value",
            huse::SerException
        );
    }

    {
        CHECK_THROWS_WITH_AS(
            JsonSerPack().node().val(std::numeric_limits<float>::infinity()),
            "Invalid JSON number value",
            huse::SerException
        );
    }

    {
        CHECK_THROWS_WITH_AS(
            JsonSerPack().node().val(std::numeric_limits<double>::quiet_NaN()),
            "Invalid JSON number value",
            huse::SerException
        );
    }
}

void testDeSimple(const huse::DeNode<huse::json::DeState>& node) {
    CHECK(node.type().isObject());
    auto obj = node.obj();
    {
        auto ar = obj.ar("array");
        CHECK(ar.size() == 4);
        int i;
        ar.val(i);
        CHECK(i == 1);
        ar.val(i);
        CHECK(i == 2);
        ar.val(i);
        CHECK(i == 3);
        double dbl;
        ar.at(0).val(dbl);
        CHECK(dbl == 1.0);
        unsigned ii;
        ar.val(ii);
        CHECK(ii == 2);
        uint16_t i16;
        ar.at(3).val(i16);
        CHECK(i16 == 4);
    }
    {
        bool b = false;
        CHECK(obj.optval("bool", b));
        CHECK(b);
    }
    {
        auto q = obj.keyval();
        CHECK(q.first == "bool2");
        CHECK(q.second.type().isBoolean());
        bool b = true;
        q.second.val(b);
        CHECK(!b);
    }
    std::string str;
    obj.val("str", str);
    CHECK(str == "b\n\\g\t\033sdf");
    CHECK(obj.done());
    int i = 8;
    std::string key = "horse";
    CHECK_FALSE(obj.optkeyval(key, i));
    CHECK(key == "horse");
    CHECK(i == 8);
    float f;
    obj.val("float", f);
    CHECK(f == 3.1f);
    i = 123;
    obj.val("int", i);
    CHECK(i == -3);
    CHECK_FALSE(obj.optval("no-such-int", i));
    CHECK(i == -3);
    unsigned long long ull;
    obj.val("unsigned-long-long", ull);
    CHECK(ull == 900000000000000);
    {
        auto inode = obj.key("int");
        CHECK(inode.type().isInteger());
        CHECK(inode.type().isNumber());
        CHECK(!inode.type().isFloat());
        inode.val(i);
        CHECK(i == -3);
    }
    {
        auto fnode = obj.key("float");
        CHECK(fnode.type().isFloat());
        CHECK(fnode.type().isNumber());
        fnode.val(f);
        CHECK(f == 3.1f);
    }
    int i2 = 0;
    obj.keyval(key, i2);
    CHECK(key == "int");
    CHECK(i2 == -3);
}

TEST_CASE("de simple") {
    {
        huse::json::DeRoot d(huse::Parse, "[]");
        auto ar = d.ar();
        CHECK(ar.size() == 0);
    }

    const std::string_view json =
        R"({"array":[1,2,3,4],"bool":true,"bool2":false,"float":3.1,"int":-3,"unsigned-long-long":900000000000000,"str":"b\n\\g\t\u001bsdf"})"
        ;

    {
        huse::json::DeRoot d(huse::Parse, json);
        testDeSimple(d);
    }
    {
        huse::json::DeRoot d(huse::Parse_takeString, std::string(json));
        testDeSimple(d);
    }
    {
        std::vector<char> buf(json.begin(), json.end());
        huse::json::DeRoot d(huse::Parse_withMutableExternalSource, buf);
        testDeSimple(d);
    }
}

TEST_CASE("de iteration") {
    {
        huse::json::DeRoot d(huse::Parse, R"({"a": 1, "b": 2, "c": 3, "d": 4})");
        auto obj = d.obj();
        CHECK(obj.size() == 4);
        {
            auto q = obj.keyval();
            CHECK(q.first == "a");
        }
        {
            auto q = obj.keyval();
            CHECK(q.first == "b");
            CHECK(q.second.type().isNumber());
            int n;
            q.second.val(n);
            CHECK(n == 2);
        }
        int n;
        std::string_view ksv;
        obj.keyval(ksv, n);
        CHECK(ksv == "c");
        CHECK(n == 3);
        std::string kstr;
        obj.keyval(kstr, n);
        CHECK(kstr == "d");
        CHECK(n == 4);
        CHECK(obj.done());
        {
            auto q = obj.keyval();
            CHECK_FALSE(q.second);
        }

        obj.resetIteration();
        std::vector<std::pair<std::string, int>> pairs;
        for (auto [key, val] : obj) {
            int i;
            val.val(i);
            pairs.emplace_back(key, i);
        }
        CHECK(pairs == std::vector<std::pair<std::string, int>>{{"a", 1}, { "b", 2 }, { "c", 3 }, { "d", 4 }});
    }

    {
        huse::json::DeRoot d(huse::Parse, R"([10, true, "xx"])");
        auto ar = d.ar();
        CHECK(ar.size() == 3);
        int n;
        {
            auto q = ar.val();
            REQUIRE(q);
            CHECK(q.type().isNumber());
            q.val(n);
            CHECK(n == 10);
        }
        bool b;
        ar.val(b);
        CHECK(b);
        std::string_view s;
        ar.val(s);
        CHECK(s == "xx");
        CHECK(ar.done());
        CHECK_FALSE(ar.val());

        ar.resetIndex();
        std::vector<pojobuf::value> vals;
        for (const auto& node : ar) {
            vals.push_back(node._state().top().value);
        }
        REQUIRE(vals.size() == 3);
        CHECK(vals[0].integer_value() == 10);
        CHECK(vals[1].type().is_true());
        CHECK(vals[2].string_value() == "xx");
    }
}

#define CHECK_THROWS_D(e, txt) CHECK_THROWS_WITH_AS(e, txt, huse::DeException)

TEST_CASE("de exceptions") {
    {
        CHECK_THROWS_AS(huse::json::DeRoot(huse::Parse, "{"), huse::DeParseException);
    }

    constexpr std::string_view json = R"({"ar": [2.3, {"x": 1, "y": 3.3}, -5], "val": 5, "b": false})";
    REQUIRE_NOTHROW(huse::json::DeRoot(huse::Parse, json));

    bool b;
    int32_t i32;
    int64_t i64;
    uint32_t u32;
    float f;
    std::string_view str;
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.val(b), "root<object>: not a boolean");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.val(i32), "root<object>: not an integer");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.val(i64), "root<object>: not an integer");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.obj().ar("ar").val(i64), R"(root."ar"[0]<float>: not an integer)");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.obj().ar("ar").at(2).val(u32), R"(root."ar"[2]<integer>: negative integer)");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.val(f), "root<object>: not a number");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.val(str), "root<object>: not a string");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.ar(), "root<object>: not an array");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.obj().obj("ar"), R"(root."ar"<array>: not an object)");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.obj().ar("ar").at(5).val(u32), R"(root."ar"[5]<undefined>: not an integer)");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        CHECK_THROWS_D(d.obj().key("zzz").ar(), R"(root."zzz"<undefined>: not an array)");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        auto o = d.obj();
        auto a = o.ar("ar");
        a.at(2).val(i32);
        CHECK_THROWS_D(a.val(str), R"(root."ar"[3]<undefined>: not a string)");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        auto o = d.obj();
        o.val("b", b);
        std::string_view key;
        CHECK_THROWS_D(o.keyval(key, b), R"(root.""<undefined>: not a boolean)");
    }
    {
        huse::json::DeRoot d(huse::Parse, json);
        auto o = d.obj();
        auto a = o.ar("ar");
        auto io = a.at(1).obj();
        auto rf = [](const huse::DeNode<huse::json::DeState>& n, float& out) {
            n.val(out);
            if (out > 2) n.throwException("val too big");
        };
        CHECK_NOTHROW(io.cval("x", f, rf));
        CHECK(f == 1);
        CHECK_THROWS_D(io.cval("y", f, rf), R"(root."ar"[1]."y"<float>: val too big)");
    }
}

TEST_CASE("string roundtrip") {
    std::string zeroStart = "0starts with zero";
    zeroStart[0] = 0;
    std::string midZero = R"(C:\Windows\foo\n\tIndented text\nSomething "else")";
    midZero += '\0';
    midZero += "After zero";
    const std::vector<std::string_view> vec = {
        "",
        "simple string",
        midZero,
        zeroStart,
        "\"quoted string\"",
        "windows newline\r\n",
        "something\b\t\ff\tu\nn\rk\fy\nor other\t\t\t",
        (const char*)(
        u8"\u0417\u0434\u0440\u0430\u0432\u0435\u0439\u002c\u0020\u0441\u0432\u044f"
        u8"\u0442\u0021\u000d\u000a\u662f\u6307\u5728\u96fb\u8166\u87a2\u5e55\u986f"
        u8"\u793a\u000d\u000a\u03a0\u03c1\u03cc\u03b3\u03c1\u03b1\u03bc\u03bc\u03b1"
        u8"\u0020\u0022\u0068\u0065\u006c\u006c\u006f\u0020\u0077\u006f\u0072\u006c"
        u8"\u0064\u0022\u000d\u000a\u30cf\u30ed\u30fc\u30fb\u30ef\u30fc\u30eb\u30c9"
        u8"\U0001f34c"
        ),
    };

    JsonSerTester j;
    j.compact().val(vec);

    auto json = j.str();

    std::vector<std::string> copy;
    huse::json::DeRoot(huse::Parse, json).val(copy);

    REQUIRE(vec.size() == copy.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        CHECK(vec[i] == copy[i]);
    }
}

TEST_CASE("double roundtrip") {
    std::vector<double> nums = {
        1, -3, -0.25, 1e-10, 1e60, 1e-120, // easy
        1.65, 0.3, 0.333, 3.141592, 3e-121, //tricky
        27.900001108646396, 0.9689776221127033 // super tricky
    };

    JsonSerTester j;
    j.compact().val(nums);
    auto json = j.str();
    CHECK(json == "[1,-3,-0.25,1e-10,1e+60,1e-120,1.65,0.3,0.333,3.141592,3e-121,27.900001108646396,0.9689776221127033]");

    auto root = huse::json::DeRoot(huse::Parse, json);
    REQUIRE(root.type().isArray());

    auto ar = root.ar();
    CHECK(ar.size() == std::size(nums));
    for (uint32_t i = 0; i < ar.size(); ++i) {
        double d;
        ar.val(d);
        CHECK(d == nums[i]);
    }
}

struct BigIntegers {
    int32_t min32;
    int32_t max32;
    uint32_t maxu32;
    uint32_t pad = 0; // guarantee that padding bytes don't mess up memcmp
    int64_t i64_d;
    uint64_t u64_d;
};

template <typename N, typename B>
void serializeBI(N& n, B& b) {
    auto ar = n.ar();
    ar.val(b.min32);
    ar.val(b.max32);
    ar.val(b.maxu32);
    ar.val(b.i64_d);
    ar.val(b.u64_d);
}

TEST_CASE("limit roundtrip") {
    BigIntegers bi = {
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max(),
        std::numeric_limits<uint32_t>::max(),
        0,
        -9007199254730992ll,
        9007199254730992ull,
    };

    JsonSerTester j;
    {
        auto root = j.compact();
        serializeBI(root, bi);
    }

    const auto json = j.str();
    CHECK(json == "[-2147483648,2147483647,4294967295,-9007199254730992,9007199254730992]");

    BigIntegers cc;
    {
        huse::json::DeRoot d(huse::Parse, json);
        serializeBI(d, cc);
    }

    CHECK(std::memcmp(&bi, &cc, sizeof(BigIntegers)) == 0);
}

struct SimpleTest {
    int x;
    std::string y;
    float z;

    template <typename Obj, typename Self>
    static void huse_serdeFlat(const Obj& o, Self& self) {
        o.val("x", self.x);
        o.val("y", self.y);
        o.val("z", self.z);
    }

    template <typename Node, typename Self>
    static void huse_serde(const Node& node, Self& self) {
        huse_serdeFlat(node.obj(), self);
    }

    bool operator==(const SimpleTest&) const = default;
};

struct ComplexTest {
    SimpleTest a;
    int b;

    template <typename Node, typename Self>
    static void customSerde(const Node& node, Self& self) {
        auto o = node.obj();
        o.val("a", self.a);
        o.val("b", self.b);
    }

    bool operator==(const ComplexTest&) const = default;
};

template <typename Ser>
void huse_serde(const huse::SerNode<Ser>& n, const ComplexTest& ct) {
    ComplexTest::customSerde(n, ct);
}

void huse_serde(const huse::DeNode<huse::PojobufDeState>& n, ComplexTest& ct) {
    ComplexTest::customSerde(n, ct);
}

TEST_CASE("struct roundtrip") {
    const ComplexTest src = { {334, std::string("hello"), 4.4f}, 7 };

    JsonSerTester j;
    j.pretty().val(src);

    ComplexTest cc;

    {
        huse::json::DeRoot d(huse::Parse, j.str());
        d.val(cc);
    }

    CHECK(src == cc);

    {
        auto root = j.pretty();
        auto o = root.obj();
        o.val("something", 43);
        o.flatval(src.a);
    }

    SimpleTest scc;
    int icc;
    {
        huse::json::DeRoot d(huse::Parse, j.str());
        auto o = d.obj();
        o.flatval(scc);
        o.val("something", icc);
    }

    CHECK(icc == 43);
    CHECK(scc == src.a);
}

TEST_CASE("std::vector roundtrip") {
    const std::vector<ComplexTest> src = {
        {{334, std::string("hello"), 4.4f}, 7},
        {{13,  std::string("asd"),   7.f},  17},
        {{345, std::string("bye"),  17.f},  99},
    };

    JsonSerTester j;
    j.pretty().val(src);

    std::vector<ComplexTest> cc;
    {
        huse::json::DeRoot d(huse::Parse, j.str());
        d.val(cc);
    }
    CHECK(src == cc);
}

void serializeInt64AsMaybeString(const huse::SerNode<huse::json::SerState>& n, uint64_t i) {
    if (i < huse::json::Max_Uint64) n.val(i);
    else n.val(std::to_string(i));
}

void serializeInt64AsMaybeString(const huse::DeNode<huse::json::DeState>& n, uint64_t& i) {
    if (n.type().isNumber()) {
        n.val(i);
    }
    else {
        std::string_view str;
        n.val(str);
        i = std::strtoull(str.data(), nullptr, 10);
    }
}

struct Visitable {
    int a;
    std::string b;

    template <typename Self, typename Visitor>
    static void visitFields(Self& s, Visitor&& v) {
        v("a", s.a);
        v("b", s.b);
    }

    bool operator==(const Visitable&) const = default;
};

struct CustomSerialization {
    uint64_t a64;
    uint64_t b64;
    Visitable visitable;

    template <typename N, typename Self>
    static void huse_serde(N& n, Self& self)
    {
        auto obj = n.obj();
        auto ifunc = [](auto& n, auto& i) { serializeInt64AsMaybeString(n, i); };
        obj.cval("a64", self.a64, ifunc);
        obj.cval("b64", self.b64, ifunc);
        obj.cval("vi", self.visitable, [](auto& n, auto& v) {
            auto obj = n.obj();
            Visitable::visitFields(v, [&obj](auto& key, auto& val) {
                obj.val(key, val);
            });
        });
    }

    bool operator==(const CustomSerialization&) const = default;
};

TEST_CASE("custom serialization roundtrip") {
    CustomSerialization cs = {10'000'000'000'000'000'000ull, 1234ull, {25, "xxx"}};

    JsonSerTester j;
    j.compact().val(cs);

    const auto json = j.str();
    CHECK(json == R"({"a64":"10000000000000000000","b64":1234,"vi":{"a":25,"b":"xxx"}})");

    CustomSerialization cc;
    {
        huse::json::DeRoot d(huse::Parse, json);
        d.val(cc);
    }

    CHECK(cs == cc);
}
