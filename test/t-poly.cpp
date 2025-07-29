// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <huse/json/Deserializer.hpp>
#include <huse/json/Serializer.hpp>

#include <huse/DeclareMsg.hpp>
#include <huse/DefineMsg.hpp>
#include <huse/Domain.hpp>
#include <huse/PolyTraits.hpp>

#include <dynamix/declare_mixin.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/mutate.hpp>

#include <sstream>

#include <doctest/doctest.h>

struct PolySerializable {
    int a;
    std::string b;

    template <typename S, typename PS>
    static void serializeT(S& s, PS& self) {
        auto n = s.node();
        auto o = n.obj();
        o.val("a", self.a);
        o.val("b", self.b);
    }
};

HUSE_SD_MSG(HUSE_NO_EXPORT, PolySerializable, PolySerializable);

DYNAMIX_DECLARE_MIXIN(struct SDEx);

struct JsonSerializeTester {
    struct Pack {
        std::ostringstream sout;
        std::optional<huse::Serializer> s;

        Pack() {
            s.emplace(huse::json::Make_Serializer(sout));
        }

        std::string str() {
            s.reset();
            return sout.str();
        }
    };

    std::optional<Pack> pack;

    huse::Serializer& make() {
        HUSE_ASSERT_INTERNAL(!pack);
        pack.emplace();
        return *pack->s;
    }

    std::string str() {
        std::string ret = pack->str();
        pack.reset();
        return ret;
    }
};

TEST_CASE("poly i/o")
{
    static_assert(huse::impl::HasPolySerialize<PolySerializable>::value);
    static_assert(huse::impl::HasPolyDeserialize<PolySerializable>::value);
    PolySerializable orig = {72, "xyz"};
    JsonSerializeTester j;
    j.make().root().val(orig);

    auto json = j.str();
    CHECK(json == R"({"a":72,"b":"xyz"})");

    PolySerializable cc;
    huse::json::Make_Deserializer(json).root().val(cc);

    CHECK(orig.a == cc.a);
    CHECK(orig.b == cc.b);

    j.make();
    mutate(*j.pack->s, dynamix::add<SDEx>());
    j.pack->s->root().val(orig);

    json = j.str();
    CHECK(json == R"({"aa":7200,"bb":"xyz_"})");

    PolySerializable cc2;
    {
        auto d = huse::json::Make_Deserializer(json);
        mutate(d, dynamix::add<SDEx>());
        d.root().val(cc2);
    }

    CHECK(orig.a == cc2.a);
    CHECK(orig.b == cc2.b);
}

struct SDEx {
    void husePolySerialize(const PolySerializable& ps) {
        auto self = huse_s_self;
        auto n = self->node();
        auto o = n.obj();
        o.val("aa", ps.a * 100);
        o.val("bb", ps.b + "_");
    }
    void husePolyDeserialize(PolySerializable& ps) {
        auto self = huse_d_self;
        auto n = self->node();

        auto o = n.obj();
        o.val("aa", ps.a);
        CHECK(ps.a % 100 == 0);
        ps.a /= 100;

        o.val("bb", ps.b);
        CHECK(ps.b.length() >= 1);
        CHECK(ps.b.back() == '_');
        ps.b.pop_back();
    }
};

DYNAMIX_DEFINE_MIXIN(huse::Domain, SDEx)
    .implements<husePolySerialize_PolySerializable>()
    .implements<husePolyDeserialize_PolySerializable>()
;

HUSE_DEFINE_S_MSG_EX(const PolySerializable&, PolySerializable, true, (PolySerializable::serializeT<huse::Serializer, const PolySerializable>));
HUSE_DEFINE_D_MSG_EX(PolySerializable&, PolySerializable, true, (PolySerializable::serializeT<huse::Deserializer, PolySerializable>));
