// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "Fwd.hpp"
#include "Type.hpp"
#include "SerdeConcepts.hpp"
#include <string>
#include <string_view>
#include <cassert>
#include <utility>
#include <splat/inline.h>

namespace huse {

template <typename De> class DeArray;
template <typename De> class DeObject;

template <typename De>
class DeNode {
protected:
    De& m_state;
    bool m_ownsTop;
#if !defined(NDEBUG)
    uintptr_t m_entryId;
    void assertIsTop() const noexcept {
        assert(m_entryId == m_state.topId());
    }
#else
    FORCE_INLINE void assertIsTop() const noexcept {}
#endif

    template <typename De2> friend class DeNode;
public:
    De& _state() const noexcept { return m_state; }

    explicit DeNode(De& state, bool ownsTop = false) noexcept
        : m_state(state)
        , m_ownsTop(ownsTop)
#if !defined(NDEBUG)
        , m_entryId(state.topId())
#endif
    {}

    DeNode(const DeNode& other) noexcept
        : DeNode(other.m_state, false)
    {}
    template <std::derived_from<De> De2>
    DeNode(const DeNode<De2>& other) noexcept
        : DeNode(other.m_state, false)
    {}
    DeNode& operator=(const DeNode&) = delete;

    DeNode(DeNode&& other) noexcept
        : DeNode(other.m_state, std::exchange(other.m_ownsTop, false))
    {}
    template <std::derived_from<De> De2>
    DeNode(DeNode<De2>&& other) noexcept
        : DeNode(other.m_state, std::exchange(other.m_ownsTop, false))
    {}
    DeNode& operator=(DeNode&& other) = delete;

    ~DeNode() {
        if (m_ownsTop) {
            m_state.pop();
        }
    }

    [[noreturn]] void throwException(std::string msg) const {
        m_state.throwException(std::move(msg));
    }

    explicit operator bool() const noexcept {
        assertIsTop();
        return !m_state.topIsUndefined();
    }

    Type type() const noexcept {
        assertIsTop();
        return m_state.topType();
    }

    template <typename T>
    void val(T& out) const {
        if constexpr (impl::HasStateSerde<T, De>) {
            huseState_serde(m_state, out);
        }
        else if constexpr (impl::HasSerdeStaticFunc<T, DeNode>) {
            T::huse_serde(*this, out);
        }
        else if constexpr (impl::HasSerdeMethod<T, DeNode>) {
            out.huse_serde(*this);
        }
        else if constexpr (impl::HasSerdeFreeFunc<T, DeNode>) {
            huse_serde(*this, out);
        }
        else {
            huse_cannot_deserialize(out);
        }
    }

    template <typename T, typename F>
    void cval(T& v, F&& f) const {
        std::forward<F>(f)(*this, v);
    }

    DeArray<De> ar() const &;
    DeObject<De> obj() const &;
    DeArray<De> ar() &&;
    DeObject<De> obj() &&;

    template <typename O>
    decltype(auto) open(O&& o) const& {
        return huse_open(*this, std::forward<O>(o));
    }

    template <typename O>
    decltype(auto) open(O&& o) && {
        return huse_open(std::move(*this), std::forward<O>(o));
    }

    // SerNode compat
    FORCE_INLINE void renderCompact() {}
};

template <typename De>
class DeArray : public DeNode<De> {
public:
    using Node = DeNode<De>;

    explicit DeArray(De& state, bool ownsTop = false)
        : Node(state, ownsTop)
    {
        state.ensureTopIsArray();
    }

    DeArray(const DeArray&) noexcept = default;
    template <std::derived_from<De> De2>
    DeArray(const DeArray<De2>& other) noexcept
        : Node(other)
    {}

    DeArray(DeArray&&) noexcept = default;
    template <std::derived_from<De> De2>
    DeArray(DeArray<De2>&& other) noexcept
        : Node(std::move(other))
    {}

    Type type() const noexcept {
        return Type::Array;
    }

    uint32_t size() const noexcept {
        this->assertIsTop();
        return this->m_state.topArraySize();
    }
    bool done() const noexcept {
        this->assertIsTop();
        return this->m_state.topArrayDone();
    }
    void resetIndex() const noexcept {
        this->assertIsTop();
        this->m_state.topArrayResetIndex();
    }

    Node at(uint32_t index) const noexcept {
        this->assertIsTop();
        this->m_state.topArrayPushElement(index);
        return Node(this->m_state, true);
    }
    Node val() const noexcept {
        this->assertIsTop();
        this->m_state.topArrayPushNextElement();
        return Node(this->m_state, true);
    }

    template <typename T>
    void val(T& out) const {
        val().val(out);
    }
    template <typename T, typename F>
    void cval(T& v, F&& f) const {
        val().cval(v, std::forward<F>(f));
    }

    DeArray ar() const {
        return val().ar();
    }
    DeObject<De> obj() const {
        return val().obj();
    }

    class Iterator {
        const DeArray& m_ar;
        union { Node m_node; };
        bool m_done;

        void next() {
            m_done = m_ar.done();
            new (&m_node) Node(m_ar.val());
        }
    public:
        explicit Iterator(const DeArray& ar) noexcept
            : m_ar(ar)
        {
            next();
        }

        ~Iterator() {
            m_node.~Node();
        }

        const Node& operator*() const noexcept {
            return m_node;
        }
        Iterator& operator++() noexcept {
            m_node.~Node();
            next();
            return *this;
        }
        bool done() const noexcept {
            return m_done;
        }

        using end_t = std::default_sentinel_t;
        friend bool operator==(const Iterator& i, const end_t&) { return i.done(); }
        friend bool operator==(const end_t&, const Iterator& i) { return i.done(); }
        friend bool operator!=(const Iterator& i, const end_t&) { return !i.done(); }
        friend bool operator!=(const end_t&, const Iterator& i) { return !i.done(); }
    };
    Iterator begin() const {
        return Iterator(*this);
    }
    static Iterator::end_t end() {
        return {};
    }
};

template <typename De>
class DeObject : public DeNode<De> {
public:
    using Node = DeNode<De>;

    explicit DeObject(De& state, bool ownsTop = false)
        : Node(state, ownsTop)
    {
        state.ensureTopIsObject();
    }

    DeObject(const DeObject&) noexcept = default;
    template <std::derived_from<De> De2>
    DeObject(const DeObject<De2>& other) noexcept
        : Node(other)
    {}

    DeObject(DeObject&&) noexcept = default;
    template <std::derived_from<De> De2>
    DeObject(DeObject<De2>&& other) noexcept
        : Node(std::move(other))
    {}

    Type type() const noexcept {
        return Type::Object;
    }

    uint32_t size() const noexcept {
        this->assertIsTop();
        return this->m_state.topObjectSize();
    }
    bool done() const noexcept {
        this->assertIsTop();
        return this->m_state.topObjectDone();
    }
    void resetIteration() const noexcept {
        this->assertIsTop();
        this->m_state.topObjectResetIteration();
    }

    using KvPair = std::pair<std::string_view, Node>;

    KvPair keyval() const noexcept {
        this->assertIsTop();
        auto key = this->m_state.topObjectPushNextKey();
        return {key, Node(this->m_state, true)};
    }

    template <typename Key, typename T>
    void keyval(Key& k, T& v) const {
        auto [key, node] = keyval();
        k = Key(key);
        node.val(v);
    }

    template <typename Key, typename T>
    bool optkeyval(Key& k, T& v) const {
        auto [key, node] = keyval();
        if (!node) {
            return false;
        }
        k = Key(key);
        node.val(v);
        return true;
    }

    Node key(std::string_view k) const noexcept {
        this->assertIsTop();
        this->m_state.topObjectPushKey(k);
        return Node(this->m_state, true);
    }

    template <typename T>
    void val(std::string_view k, T& out) const {
        key(k).val(out);
    }
    template <typename T, typename F>
    void cval(std::string_view k, T& v, F&& f) const {
        key(k).cval(v, std::forward<F>(f));
    }

    template <typename T>
    bool optval(std::string_view k, T& v) const {
        auto node = key(k);
        if (!node) {
            return false;
        }
        node.val(v);
        return true;
    }

    template <typename T>
    void flatval(T& v) const {
        if constexpr (impl::HasSerdeFlatStaticFunc<T, DeObject>) {
            T::huse_serdeFlat(*this, v);
        }
        else if constexpr (impl::HasSerdeFlatMethod<T, DeObject>) {
            v.huse_serdeFlat(*this);
        }
        else if constexpr (impl::HasSerdeFlatFreeFunc<T, DeObject>) {
            huse_serdeFlat(*this, v);
        }
        else {
            huse_cannot_deserialize_flat(v);
        }
    }

    DeArray<De> ar(std::string_view k) const {
        return key(k).ar();
    }
    DeObject<De> obj(std::string_view k) const {
        return key(k).obj();
    }

    template <typename O>
    decltype(auto) open(O&& o) const& {
        return huse_open(*this, std::forward<O>(o));
    }

    template <typename O>
    decltype(auto) open(O&& o) && {
        return huse_open(std::move(*this), std::forward<O>(o));
    }

    class Iterator {
        const DeObject& m_obj;
        union { KvPair m_pair; };
        bool m_done;

        void next() {
            m_done = m_obj.done();
            new (&m_pair) KvPair(m_obj.keyval());
        }
    public:
        explicit Iterator(const DeObject& obj) noexcept
            : m_obj(obj)
        {
            next();
        }

        ~Iterator() {
            m_pair.~KvPair();
        }

        const KvPair& operator*() const noexcept {
            return m_pair;
        }
        Iterator& operator++() noexcept {
            m_pair.~KvPair();
            next();
            return *this;
        }
        bool done() const noexcept {
            return m_done;
        }

        using end_t = std::default_sentinel_t;
        friend bool operator==(const Iterator& i, const end_t&) { return i.done(); }
        friend bool operator==(const end_t&, const Iterator& i) { return i.done(); }
        friend bool operator!=(const Iterator& i, const end_t&) { return !i.done(); }
        friend bool operator!=(const end_t&, const Iterator& i) { return !i.done(); }
    };
    Iterator begin() const {
        return Iterator(*this);
    }
    static Iterator::end_t end() {
        return {};
    }
};

template <typename De>
DeArray<De> DeNode<De>::ar() const& {
    return DeArray<De>(m_state, false);
}

template <typename De>
DeObject<De> DeNode<De>::obj() const& {
    return DeObject<De>(m_state, false);
}

template <typename De>
DeArray<De> DeNode<De>::ar()&& {
    return DeArray<De>(m_state, std::exchange(m_ownsTop, false));
}

template <typename De>
DeObject<De> DeNode<De>::obj()&& {
    return DeObject<De>(m_state, std::exchange(m_ownsTop, false));
}

} // namespace huse
