// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <huse/json/Deserializer.hpp>
#include <huse/json/Serializer.hpp>
#include <huse/CtxDomain.hpp>

#include <dynamix/declare_mixin.hpp>
#include <dynamix/define_mixin.hpp>
#include <dynamix/msg/declare_msg.hpp>
#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/msg_traits.hpp>
#include <dynamix/mutate.hpp>

#include <sstream>

#include <doctest/doctest.h>

struct PolySerializable {
    int a;
    std::string b;

    template <typename N, typename PS>
    static void defaultSerialize(N& n, PS& self) {
        auto o = n.obj();
        o.val("a", self.a);
        o.val("b", self.b);
    }

    struct Io {
        virtual void operator()(huse::SerializerNode& n, const PolySerializable& ps) const = 0;
        virtual void operator()(huse::DeserializerNode& n, PolySerializable& ps) const = 0;
    };

    struct Serialize;
};

DYNAMIX_DECLARE_SIMPLE_MSG(getPolySerializableIo, const PolySerializable::Io*(const huse::CtxObj&));

struct PolySerializable::Serialize {
    template <typename N, typename PS>
    void operator()(N& n, PS& ps) const {
        auto& ctx = n.ctx();
        auto* io = getPolySerializableIo::call(ctx);
        if (io) {
            (*io)(n, ps);
        }
        else {
            PS::defaultSerialize(n, ps);
        }
    }
};

DYNAMIX_DECLARE_MIXIN(struct SDEx);

TEST_CASE("poly i/o")
{
    const PolySerializable orig = {72, "xyz"};
    PolySerializable::Serialize helper;
    std::ostringstream sout;
    huse::json::Make_Serializer(sout).cval(orig, helper);

    auto json = sout.str();
    CHECK(json == R"({"a":72,"b":"xyz"})");

    PolySerializable cc;
    huse::json::Make_Deserializer(json).cval(cc, helper);

    CHECK(orig.a == cc.a);
    CHECK(orig.b == cc.b);

    {
        auto s = huse::json::Make_Serializer(sout);
        mutate(s.ctx(), dynamix::add<SDEx>());
        s.cval(orig, helper);
    }

    json = sout.str();
    CHECK(json == R"({"aa":7200,"bb":"xyz_"})");

    PolySerializable cc2;
    {
        auto d = huse::json::Make_Deserializer(json);
        mutate(d.ctx(), dynamix::add<SDEx>());
        d.cval(cc2, helper);
    }

    CHECK(orig.a == cc2.a);
    CHECK(orig.b == cc2.b);
}

struct SDEx {
    struct Io final : public PolySerializable::Io {
        void operator()(huse::SerializerNode& n, const PolySerializable& ps) const override {
            auto o = n.obj();
            o.val("aa", ps.a * 100);
            o.val("bb", ps.b + "_");
        }
        void operator()(huse::DeserializerNode& n, PolySerializable& ps) const override {
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
    Io m_io;

    static const PolySerializable::Io* get(const SDEx* self) {
        return &self->m_io;
    }
};

DYNAMIX_DEFINE_MIXIN(huse::CtxDomain, SDEx)
    .implements_by<getPolySerializableIo>(&SDEx::get)
;

DYNAMIX_DEFINE_SIMPLE_MSG_EX(getPolySerializableIo, unicast, false,
    [](const huse::CtxObj&)->const PolySerializable::Io* {
        return nullptr;
    }
);
