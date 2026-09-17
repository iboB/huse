// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Fwd.hpp"
#include "SerdeConcepts.hpp"
#include <string>
#include <string_view>
#include <cassert>
#include <utility>
#include <splat/inline.h>

namespace huse {

template <typename Ser> class SerObject;
template <typename Ser> class SerArray;

template <typename Ser>
class SerNode {
protected:
    Ser& m_state;
    template <typename Ser2> friend class SerNode;

public:
    explicit SerNode(Ser& state) noexcept
        : m_state(state)
    {}

    Ser& _state() const noexcept { return m_state; }

    SerNode(const SerNode& other) noexcept
        : SerNode(other.m_state)
    {}
    template <std::derived_from<Ser> Ser2>
    SerNode(const SerNode<Ser2>& other) noexcept
        : SerNode(other.m_state)
    {}
    SerNode& operator=(const SerNode&) = delete;

    [[noreturn]] void throwException(std::string msg) const {
        m_state.throwException(std::move(msg));
    }

    template <typename T>
    void val(T&& t) const {
        using VT = std::decay_t<T>;

        if constexpr (impl::HasStateSerde<VT, Ser>) {
            huseState_serde(m_state, std::forward<T>(t));
        }
        else if constexpr (impl::HasSerdeStaticFunc<VT, SerNode>) {
            VT::huse_serde(*this, std::forward<T>(t));
        }
        else if constexpr (impl::HasSerdeMethod<VT, SerNode>) {
            std::forward<T>(t).huse_serde(*this);
        }
        else if constexpr (impl::HasSerdeFreeFunc<VT, SerNode>) {
            huse_serde(*this, std::forward<T>(t));
        }
        else {
            huse_cannot_serialize(t);
        }
    }

    template <typename V, typename F>
    void cval(V&& v, F&& f) const {
        std::forward<F>(f)(*this, std::forward<V>(v));
    }

    void renderCompact() {
        m_state.setRenderCompact();
    }

    void discard() const noexcept {
        m_state.discardTop();
    }

    SerArray<Ser> ar() const;
    SerObject<Ser> obj() const;

    template <typename O>
    decltype(auto) open(O&& o) const {
        return huse_open(*this, std::forward<O>(o));
    }

    // DeNode compat
    explicit constexpr operator bool() const noexcept { return true; }
};

template <typename Ser>
class SerCompound : protected SerNode<Ser> {
protected:
    template <typename Ser2> friend class SerCompound;

    bool m_ownsClose;
#if !defined(NDEBUG)
    uint32_t m_depth;
    void assertIsTop() const noexcept {
        assert(m_depth == this->m_state.curStackDepth());
    }
#else
    FORCE_INLINE void assertIsTop() const noexcept {}
#endif
    SerCompound(Ser& state, bool ownsClose)
        : SerNode<Ser>(state)
        , m_ownsClose(ownsClose)
#if !defined(NDEBUG)
        , m_depth(state.curStackDepth())
#endif
    {}

    SerCompound(const SerCompound& other) noexcept
        : SerCompound(other.m_state, false)
    {
#if !defined(NDEBUG)
        m_depth = other.m_depth;
#endif
    }
    template <std::derived_from<Ser> Ser2>
    SerCompound(const SerCompound<Ser2>& other) noexcept
        : SerCompound(other.m_state, false)
    {
#if !defined(NDEBUG)
        m_depth = other.m_depth;
#endif
    }

    SerCompound(SerCompound&& other) noexcept
        : SerCompound(other.m_state, std::exchange(other.m_ownsClose, false))
    {
#if !defined(NDEBUG)
        m_depth = other.m_depth;
#endif
    }

    template <std::derived_from<Ser> Ser2>
    SerCompound(SerCompound<Ser2>&& other) noexcept
        : SerCompound(other.m_state, std::exchange(other.m_ownsClose, false))
    {
#if !defined(NDEBUG)
        m_depth = other.m_depth;
#endif
    }

    SerCompound& operator=(SerCompound&&) = delete;

public:
    using SerNode<Ser>::_state;
    using SerNode<Ser>::renderCompact;
};

template <typename Ser>
class SerArray : public SerCompound<Ser> {
public:
    using Node = SerNode<Ser>;
    using Super = SerCompound<Ser>;

    ~SerArray() {
        if (this->m_ownsClose) {
            this->assertIsTop();
            this->m_state.topArrayClose();
        }
    }

    SerArray(const SerArray&) noexcept = default;
    template <std::derived_from<Ser> Ser2>
    SerArray(const SerArray<Ser2>& other) noexcept
        : Super(other)
    {}

    SerArray(SerArray&&) noexcept = default;
    template <std::derived_from<Ser> Ser2>
    SerArray(SerArray<Ser2>&& other) noexcept
        : Super(std::move(other))
    {}

    using Super::val;
    using Super::cval;
    using Super::ar;
    using Super::obj;

    FORCE_INLINE const Node& val() const { return *this; }

private:
    explicit SerArray(Ser& state)
        : Super(state, true)
    {}
    friend Node;
};

template <typename Ser>
class SerObject : public SerCompound<Ser> {
public:
    using Node = SerNode<Ser>;
    using Super = SerCompound<Ser>;

    ~SerObject() {
        if (this->m_ownsClose) {
            this->assertIsTop();
            this->m_state.topObjectClose();
        }
    }

    SerObject(const SerObject&) noexcept = default;
    template <std::derived_from<Ser> Ser2>
    SerObject(const SerObject<Ser2>& other) noexcept
        : Super(other)
    {}

    SerObject(SerObject&&) noexcept = default;
    template <std::derived_from<Ser> Ser2>
    SerObject(SerArray<Ser2>&& other) noexcept
        : Super(std::move(other))
    {}

    const Node& key(std::string_view k) const {
        this->assertIsTop();
        this->m_state.topObjectPushKey(k);
        return *this;
    }

    SerObject obj(std::string_view k) const {
        return key(k).obj();
    }
    SerArray<Ser> ar(std::string_view k) const {
        return key(k).ar();
    }

    template <typename V>
    void val(std::string_view k, V&& v) const {
        key(k).val(std::forward<V>(v));
    }

    template <typename V, typename F>
    void cval(std::string_view k, V&& v, F&& f) const {
        key(k).cval(std::forward<V>(v), std::forward<F>(f));
    }

    template <typename T>
    void flatval(T&& v) const {
        using VT = std::decay_t<T>;

        if constexpr (impl::HasSerdeFlatStaticFunc<VT, SerObject>) {
            VT::huse_serdeFlat(*this, v);
        }
        else if constexpr (impl::HasSerdeFlatMethod<VT, SerObject>) {
            v.huse_serdeFlat(*this);
        }
        else if constexpr (impl::HasSerdeFlatFreeFunc<VT, SerObject>) {
            huse_serdeFlat(*this, v);
        }
        else {
            huse_cannot_deserialize_flat(v);
        }
    }

    // DeObject compat
    template <typename V>
    bool optval(std::string_view k, V&& v) const {
        val(k, std::forward<V>(v));
        return true;
    }
    template <typename V>
    void keyval(std::string_view k, V&& v) const {
        val(k, std::forward<V>(v));
    }
    template <typename V>
    bool optkeyval(std::string_view k, V&& v) const {
        val(k, std::forward<V>(v));
        return true;
    }

    template <typename O>
    decltype(auto) open(O&& o) const {
        return huse_open(*this, std::forward<O>(o));
    }

private:
    explicit SerObject(Ser& state)
        : Super(state, true)
    {}
    friend Node;
};

template <typename Ser>
SerArray<Ser> SerNode<Ser>::ar() const {
    m_state.pushArrayValue();
    return SerArray<Ser>(m_state);
}

template <typename Ser>
SerObject<Ser> SerNode<Ser>::obj() const {
    m_state.pushObjectValue();
    return SerObject<Ser>(m_state);
}

} // namespace huse
