// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <huse/json/DeRoot.hpp>
#include <huse/json/SerRoot.hpp>

#include <huse/CtxDomain.hpp>
#include <huse/Ctx.hpp>

#include <trex/facets/declare.hpp>
#include <trex/facets/define.hpp>

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
};

struct PolySerializableFacet {
    virtual void ser(const huse::SerNode<huse::ISerState>& n, const PolySerializable& ps) const = 0;
    virtual void de(const huse::DeNode<huse::PojobufDeState>&n, PolySerializable & ps) const = 0;
};
TREX_DECLARE_FACET(huse::CtxDomain, PolySerializableFacet);

void huseState_serde(huse::ISerState& self, const PolySerializable& val) {
    huse::SerNode node(self);
    if (auto psf = self.ctx.pget<PolySerializableFacet>()) {
        psf->ser(node, val);
    }
    else {
        PolySerializable::defaultSerialize(node, val);
    }
}

void huseState_serde(huse::PojobufDeState& self, PolySerializable& out) {
    huse::DeNode node(self);
    if (auto psf = self.ctx.pget<PolySerializableFacet>()) {
        psf->de(node, out);
    }
    else {
        PolySerializable::defaultSerialize(node, out);
    }
}

struct PolySerializableFacetImpl : public PolySerializableFacet {
    void ser(const huse::SerNode<huse::ISerState>& n, const PolySerializable& ps) const override {
        auto o = n.obj();
        o.val("aa", ps.a * 100);
        o.val("bb", ps.b + "_");
    }
    void de(const huse::DeNode<huse::PojobufDeState>& n, PolySerializable& ps) const override {
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

TEST_CASE("poly roundtrip")
{
    const PolySerializable orig = {72, "xyz"};

    std::ostringstream sout;
    huse::json::SerRoot(sout).val(orig);
    auto json = sout.str();
    sout.str("");
    CHECK(json == R"({"a":72,"b":"xyz"})");

    PolySerializable cc;
    huse::json::DeRoot(huse::Parse, json).val(cc);

    CHECK(orig.a == cc.a);
    CHECK(orig.b == cc.b);

    PolySerializableFacetImpl psf;

    {
        huse::json::SerRoot s(sout);
        s.ctx.reset_ref<PolySerializableFacet>(psf);
        s.val(orig);
    }

    json = sout.str();
    CHECK(json == R"({"aa":7200,"bb":"xyz_"})");

    PolySerializable cc2;
    {
        auto d = huse::json::DeRoot(huse::Parse, json);
        d.ctx.reset_ref<PolySerializableFacet>(psf);
        d.val(cc2);
    }

    CHECK(orig.a == cc2.a);
    CHECK(orig.b == cc2.b);
}

TREX_DEFINE_FACET(huse::CtxDomain, PolySerializableFacet);
