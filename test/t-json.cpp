// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <doctest/doctest.h>

#include <huse/json/Deserializer.hpp>
#include <huse/json/Serializer.hpp>

#include <huse/helpers/StdVector.hpp>

#include <huse/Exception.hpp>

#include <sstream>
#include <limits>
#include <cstring>

TEST_SUITE_BEGIN("json");

struct JsonSerializerPack
{
    std::ostringstream sout;
    std::optional<huse::json::Serializer> s;

    JsonSerializerPack(bool pretty = false, uintptr_t ctx = 0)
    {
        s.emplace(sout, pretty, ctx);
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

    huse::json::Serializer& make(bool pretty, uintptr_t ctx)
    {
        HUSE_ASSERT_INTERNAL(!pack);
        pack.emplace(pretty, ctx);
        return *pack->s;
    }

    huse::json::Serializer& compact(uintptr_t ctx = 0) { return make(false, ctx); }
    huse::json::Serializer& pretty(uintptr_t ctx = 0) { return make(true, ctx); }

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

TEST_CASE("serializer stream")
{
    JsonSerializeTester j;

    j.compact().sstream() << "xx " << 123;
    CHECK(j.str() == R"("xx 123")");

    {
        auto s = j.compact().sstream();
        s << -5;
        s.get().put(' ');
        s.get().write("abc", 3);
    }
    CHECK(j.str() == R"("-5 abc")");

    {
        auto s = j.compact().sstream();
        s << "b\n\\g";
        s.get().put('\t');
        s.get().put(27);
        s << "sdf";
    }
    CHECK(j.str() == R"("b\n\\g\t\u001bsdf")");
}

TEST_CASE("serializer exceptions")
{
    {
        CHECK_THROWS_WITH_AS(
            JsonSerializerPack().s->val(1ull << 55),
            "Integer value is bigger than maximum allowed for JSON",
            huse::SerializerException
        );
    }

    {
        CHECK_THROWS_WITH_AS(
            JsonSerializerPack().s->val(-(1ll << 55)),
            "Integer value is bigger than maximum allowed for JSON",
            huse::SerializerException
        );
    }

    {
        CHECK_THROWS_WITH_AS(
            JsonSerializerPack().s->val(std::numeric_limits<float>::infinity()),
            "Floating point value is not finite. Not supported by JSON",
            huse::SerializerException
        );
    }

    {
        CHECK_THROWS_WITH_AS(
            JsonSerializerPack().s->val(std::numeric_limits<double>::quiet_NaN()),
            "Floating point value is not finite. Not supported by JSON",
            huse::SerializerException
        );
    }
}

huse::json::Deserializer makeD(std::string_view str, uintptr_t ctx = 0)
{
    return huse::json::Deserializer::fromConstString(str, ctx);
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
        CHECK(d.type().is(huse::Type::Object));
        auto obj = d.obj();
        CHECK(obj.type().is(huse::Type::Object));
        {
            auto ar = obj.ar("array");
            CHECK(ar.type().is(huse::Type::Array));
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
            uint16_t i16;
            ar.index(3).val(i16);
            CHECK(i16 == 4);
        }
        bool b;
        obj.val("bool", b);
        CHECK(b);

        auto q = obj.peeknext();
        CHECK(!!q);
        CHECK(q.name == "bool2");
        CHECK(q.node->type().is(huse::Type::Boolean));
        CHECK(q.node->type().is(huse::Type::False));
        q->val(b);
        CHECK(!b);

        std::string str;
        obj.val("str", str);
        CHECK(str == "b\n\\g\t\033sdf");

        CHECK(!obj.peeknext());

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
        CHECK(inode.type().is(huse::Type::Integer));
        CHECK(inode.type().is(huse::Type::Number));
        CHECK(!inode.type().is(huse::Type::Float));

        auto& fnode = obj.key("float");
        CHECK(fnode.type().is(huse::Type::Float));
        CHECK(fnode.type().is(huse::Type::Number));

        CHECK(obj.key("str").type().is(huse::Type::String));

        CHECK(obj.key("array").type().is(huse::Type::Array));

        obj.val("float", f);

        std::string key;
        int i2 = 0;
        obj.nextkeyval(key, i2);
        CHECK(key == "int");
        CHECK(i2 == -3);
    }
}

TEST_CASE("stream deserialize")
{
    auto d = makeD(R"({"string":"aa bbb c"})");
    auto o = d.obj();
    CHECK(!o.optsstream("asdf"));
    CHECK(!!o.optsstream("string"));

    {
        std::string a, b, c;
        o.sstream("string") >> a >> b >> c;
        CHECK(a == "aa");
        CHECK(b == "bbb");
        CHECK(c == "c");
    }

    {
        std::string a, b, c;
        auto s = o.optsstream("string");
        REQUIRE(!!s);
        *s >> a >> b >> c;
        CHECK(a == "aa");
        CHECK(b == "bbb");
        CHECK(c == "c");
        CHECK(s->get().eof());
    }
}

#define CHECK_THROWS_D(e, txt) CHECK_THROWS_WITH_AS(e, txt, huse::DeserializerException)

TEST_CASE("deserialize iteration")
{
    {
        auto d = makeD(R"({"a": 1, "b": 2, "c": 3, "d": 4})");
        auto obj = d.obj();
        CHECK(obj.length() == 4);
        auto q = obj.peeknext();
        CHECK(q.name == "a");
        q->skip();
        q = obj.peeknext();
        CHECK(q.name == "b");
        int n;
        q->val(n);
        CHECK(n == 2);
        std::string_view ksv;
        obj.nextkeyval(ksv, n);
        CHECK(ksv == "c");
        CHECK(n == 3);
        std::string kstr;
        obj.nextkeyval(kstr, n);
        CHECK(kstr == "d");
        CHECK(n == 4);
        CHECK(obj.end());
        CHECK(!obj.peeknext());
        CHECK_THROWS_D(obj.nextkeyval(ksv, n), "root.[4] : out of range");
    }

    {
        auto d = makeD(R"([10, true, "xx"])");
        auto ar = d.ar();
        CHECK(ar.length() == 3);
        auto q = ar.peeknext();
        CHECK(q);
        CHECK(q->type().is(huse::Type::Number));
        int n;
        q->val(n);
        CHECK(n == 10);
        ar.skip();
        std::string_view s;
        ar.val(s);
        CHECK(s == "xx");
        CHECK(ar.end());
        CHECK(!ar.peeknext());
        CHECK_THROWS_D(ar.val(n), "root.[3] : out of range");
    }
}

TEST_CASE("deserializer exceptions")
{
    {
        CHECK_THROWS_AS(makeD("{"), huse::DeserializerException);
    }

    constexpr std::string_view json = R"({"ar": [2.3, {"x": 1, "y": 3.3}, -5], "val": 5, "b": false})";
    {
        REQUIRE_NOTHROW(makeD(json));
    }

    bool b;
    int32_t i32;
    int64_t i64;
    uint32_t u32;
    float f;
    std::string_view str;
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.val(b), "root : not a boolean");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.val(i32), "root : not an integer");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.val(i64), "root : not an integer");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.obj().ar("ar").val(i64), R"(root."ar".[0] : not an integer)");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.obj().ar("ar").index(2).val(u32), R"(root."ar".[2] : negative integer)");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.val(f), "root : not a number");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.val(str), "root : not a string");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.ar(), "root : not an array");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.obj().obj("ar"), R"(root."ar" : not an object)");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.obj().ar("ar").index(5), R"(root."ar".[5] : out of range)");
    }
    {
        auto d = makeD(json);
        CHECK_THROWS_D(d.obj().key("zzz"), R"(root."zzz" : out of range)");
    }
    {
        auto d = makeD(json);
        auto o = d.obj();
        auto a = o.ar("ar");
        a.index(2).val(i32);
        CHECK_THROWS_D(a.val(str), R"(root."ar".[3] : out of range)");
    }
    {
        auto d = makeD(json);
        auto o = d.obj();
        bool b;
        o.val("b", b);
        std::string_view key;
        CHECK_THROWS_D(o.nextkeyval(key, b), "root.[3] : out of range");
    }
}

TEST_CASE("string i/o")
{
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
        u8"\u0417\u0434\u0440\u0430\u0432\u0435\u0439\u002c\u0020\u0441\u0432\u044f"
        u8"\u0442\u0021\u000d\u000a\u662f\u6307\u5728\u96fb\u8166\u87a2\u5e55\u986f"
        u8"\u793a\u000d\u000a\u03a0\u03c1\u03cc\u03b3\u03c1\u03b1\u03bc\u03bc\u03b1"
        u8"\u0020\u0022\u0068\u0065\u006c\u006c\u006f\u0020\u0077\u006f\u0072\u006c"
        u8"\u0064\u0022\u000d\u000a\u30cf\u30ed\u30fc\u30fb\u30ef\u30fc\u30eb\u30c9"
        u8"\U0001f34c",
    };

    JsonSerializeTester j;
    j.compact().val(vec);

    auto json = j.str();

    std::vector<std::string> copy;
    makeD(json).val(copy);

    REQUIRE(vec.size() == copy.size());
    for (size_t i=0; i<vec.size(); ++i)
    {
        CHECK(vec[i] == copy[i]);
    }
}

struct BigIntegers
{
    int32_t min32;
    int32_t max32;
    uint32_t maxu32;
    uint32_t pad = 0; // guarantee that padding bytes don't mess up memcmp
    int64_t i64_d;
    uint64_t u64_d;
};

template <typename N, typename B>
void serializeBI(N& n, B& b)
{
    auto ar = n.ar();
    ar.val(b.min32);
    ar.val(b.max32);
    ar.val(b.maxu32);
    ar.val(b.i64_d);
    ar.val(b.u64_d);
}

TEST_CASE("limit i/o")
{
    BigIntegers bi = {
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max(),
        std::numeric_limits<uint32_t>::max(),
        0,
        -9007199254730992ll,
        9007199254730992ull,
    };

    JsonSerializeTester j;
    serializeBI(j.compact(), bi);

    const auto json = j.str();
    CHECK(json == "[-2147483648,2147483647,4294967295,-9007199254730992,9007199254730992]");

    BigIntegers cc;
    {
        auto d = makeD(json);
        serializeBI(d, cc);
    }

    CHECK(memcmp(&bi, &cc, sizeof(BigIntegers)) == 0);
}

struct SimpleTest
{
    int x;
    std::string y;
    float z;

    template <typename O, typename Self>
    static void serializeFlatT(O& o, Self& self)
    {
        o.val("x", self.x);
        o.val("y", self.y);
        o.val("z", self.z);
    }

    void huseSerializeFlat(huse::SerializerObject& o) const
    {
        serializeFlatT(o, *this);
    }

    void huseDeserializeFlat(huse::DeserializerObject& o)
    {
        serializeFlatT(o, *this);
    }

    template <typename N, typename Self>
    static void serializeT(N& n, Self& self)
    {
        auto o = n.obj();
        serializeFlatT(o, self);
    }

    void huseSerialize(huse::SerializerNode& n) const
    {
        serializeT(n, *this);
    }

    void huseDeserialize(huse::DeserializerNode& n)
    {
        serializeT(n, *this);
    }
};

bool operator==(const SimpleTest& a, const SimpleTest& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

struct ComplexTest
{
    SimpleTest a;
    int b;

    template <typename N, typename Self>
    static void serialize(N& n, Self& self)
    {
        auto o = n.obj();
        o.val("a", self.a);
        o.val("b", self.b);
    }
};

bool operator==(const ComplexTest& a, const ComplexTest& b)
{
    return a.a == b.a && a.b == b.b;
}

void huseSerialize(huse::SerializerNode& n, const ComplexTest& ct)
{
    ComplexTest::serialize(n, ct);
}

void huseDeserialize(huse::DeserializerNode& n, ComplexTest& ct)
{
    ComplexTest::serialize(n, ct);
}

TEST_CASE("struct i/o")
{
    const ComplexTest src = {{334, std::string("hello"), 4.4f}, 7};

    JsonSerializeTester j;
    j.pretty().val(src);

    ComplexTest cc;

    {
        auto d = makeD(j.str());
        d.val(cc);
    }

    CHECK(src == cc);

    {
        auto o = j.pretty().obj();
        o.val("something", 43);
        o.flatval(src.a);
    }

    SimpleTest scc;
    int icc;
    {
        auto d = makeD(j.str());
        auto o = d.obj();
        o.flatval(scc);
        o.val("something", icc);
    }

    CHECK(icc == 43);
    CHECK(scc == src.a);
}

TEST_CASE("std::vector i/o")
{
    const std::vector<ComplexTest> src = {
        {{334, std::string("hello"), 4.4f}, 7},
        {{13,  std::string("asd"),   7.f},  17},
        {{345, std::string("bye"),  17.f},  99},
    };

    JsonSerializeTester j;
    j.pretty().val(src);

    std::vector<ComplexTest> cc;
    {
        auto d = makeD(j.str());
        d.val(cc);
    }
    CHECK(src == cc);
}

void serializeInt64AsMaybeString(huse::SerializerNode& n, uint64_t i)
{
    if (i < huse::json::Serializer::Max_Uint64) n.val(i);
    else n.val(std::to_string(i));
}

void serializeInt64AsMaybeString(huse::DeserializerNode& n, uint64_t& i)
{
    if (n.type().is(huse::Type::Number)) n.val(i);
    else
    {
        std::string_view str;
        n.val(str);
        i = std::strtoull(str.data(), nullptr, 10);
    }
}

struct Visitable
{
    int a;
    std::string b;

    template <typename Self, typename Visitor>
    static void visitFields(Self& s, Visitor& v)
    {
        v("a", s.a);
        v("b", s.b);
    }
};

struct CustomSerialization
{
    uint64_t a64;
    uint64_t b64;
    Visitable visitable;

    template <typename N, typename Self>
    static void serializeT(N& n, Self& self)
    {
        auto obj = n.obj();
        auto ifunc = [](auto& n, auto& i) { serializeInt64AsMaybeString(n, i); };
        obj.cval("a64", self.a64, ifunc);
        obj.cval("b64", self.b64, ifunc);
        obj.cval("vi", self.visitable, [](auto& n, auto& v) {
            auto obj = n.obj();
            auto func = [&obj](auto& k, auto& v) {
                obj.val(k, v);
            };
            Visitable::visitFields(v, func);
        });
    }

    void huseSerialize(huse::SerializerNode& n) const
    {
        serializeT(n, *this);
    }

    void huseDeserialize(huse::DeserializerNode& n)
    {
        serializeT(n, *this);
    }

    bool operator==(const CustomSerialization& o) const
    {
        return a64 == o.a64
            && b64 == o.b64
            && visitable.a == o.visitable.a
            && visitable.b == o.visitable.b;
    }
};

TEST_CASE("custom serialization i/o")
{
    CustomSerialization cs = {10'000'000'000'000'000'000ull, 1234ull, {25, "xxx"}};

    JsonSerializeTester j;
    j.compact().val(cs);

    const auto json = j.str();
    CHECK(json == R"({"a64":"10000000000000000000","b64":1234,"vi":{"a":25,"b":"xxx"}})");

    CustomSerialization cc;
    {
        auto d = makeD(json);
        d.val(cc);
    }

    CHECK(cs == cc);
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

struct MultipleValuesAsString
{
    std::string a;
    vector2 b;

    template <typename N, typename MVS>
    static void serializeT(N& n, MVS& self)
    {
        n.obj().sstream("data") & self.b & self.a;
    }

    void huseSerialize(huse::SerializerNode& n) const
    {
        serializeT(n, *this);
    }

    void huseDeserialize(huse::DeserializerNode& n)
    {
        serializeT(n, *this);
    }
};

TEST_CASE("stream i/o")
{
    MultipleValuesAsString mvs = {"xyz", {34, 88}};
    JsonSerializeTester j;
    j.compact().val(mvs);

    auto json = j.str();

    MultipleValuesAsString cc;
    {
        auto d = makeD(json);
        d.val(cc);
    }

    CHECK(mvs.a == cc.a);
    CHECK(mvs.b.x == cc.b.x);
    CHECK(mvs.b.y == cc.b.y);
}

struct ContextSerialization
{
    int a;
    std::string b;

    template <typename N, typename CS>
    static void serializeT(N& n, CS& self)
    {
        auto o = n.obj();
        o.val("a", self.a);
        o.val("b", self.b);
    }

    void huseSerialize(huse::SerializerNode& n) const
    {
        if (n.context().i() == 0)
        {
            serializeT(n, *this);
            return;
        }

        CHECK(n.context().i() == 43);

        auto o = n.obj();
        o.val("aa", a * 100);
        o.val("bb", b + "_");
    }

    void huseDeserialize(huse::DeserializerNode& n)
    {
        if (n.context().i() == 0)
        {
            serializeT(n, *this);
            return;
        }

        CHECK(n.context().i() == 88);

        auto o = n.obj();
        o.val("aa", a);
        CHECK(a % 100 == 0);
        a /= 100;

        o.val("bb", b);
        CHECK(b.length() >= 1);
        CHECK(b.back() == '_');
        b.pop_back();
    }
};

TEST_CASE("context-based i/o")
{
    ContextSerialization orig = {72, "xyz"};
    JsonSerializeTester j;
    j.compact().val(orig);

    auto json = j.str();
    CHECK(json == R"({"a":72,"b":"xyz"})");

    ContextSerialization cc;
    {
        auto d = makeD(json);
        d.val(cc);
    }

    CHECK(orig.a == cc.a);
    CHECK(orig.b == cc.b);

    j.compact(43).val(orig);
    json = j.str();
    CHECK(json == R"({"aa":7200,"bb":"xyz_"})");

    ContextSerialization cc2;
    {
        auto d = makeD(json, 88);
        d.val(cc2);
    }

    CHECK(orig.a == cc2.a);
    CHECK(orig.b == cc2.b);
}
