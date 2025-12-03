#pragma once
#include "OpenTags.hpp"
#include "impl/UniqueStack.hpp"
#include <concepts>
#include <string_view>
#include <initializer_list>

namespace huse {

template <typename Serializer>
class SerializerObject;
template <typename Serializer>
class SerializerArray;

template <typename Serializer>
class SerializerNode : public impl::UniqueStack {
protected:
    template <typename S>
    friend class SerializerNode;

    Serializer* m_serializer;
    bool m_ownsClose = false;
public:
    SerializerNode(Serializer& s) noexcept
        : UniqueStack(nullptr)
        , m_serializer(&s)
    {}

    template <std::derived_from<Serializer> OtherSerializer>
    SerializerNode(SerializerNode<OtherSerializer>& other, bool ownsClose = false) noexcept
        : UniqueStack(&other)
        , m_serializer(other.m_serializer)
        , m_ownsClose(ownsClose)
    {}

    SerializerNode(SerializerNode&& other) noexcept
        : UniqueStack(std::move(other))
        , m_serializer(other.m_serializer)
        , m_ownsClose(other.m_ownsClose)
    {
        other.m_serializer = nullptr;
        other.m_ownsClose = false;
    }

    template <std::derived_from<Serializer> OtherSerializer>
    SerializerNode(SerializerNode<OtherSerializer>&& other) noexcept
        : UniqueStack(std::move(other))
        , m_serializer(other.m_serializer)
        , m_ownsClose(other.m_ownsClose)
    {
        other.m_serializer = nullptr;
        other.m_ownsClose = false;
    }

    [[noreturn]] void throwException(const std::string& msg) const {
        m_serializer->throwException(msg);
    }

    bool _active() const noexcept { return !!m_serializer; }
    Serializer& _s() noexcept { return *m_serializer; }

    template <typename V>
    void val(V&& v);

    template <typename O>
    decltype(auto) open(O&& o);

    template <typename V, typename F>
    void cval(V&& v, F&& f) {
        f(*this, std::forward<V>(v));
    }

    SerializerArray<Serializer> ar();
    SerializerObject<Serializer> obj();
};

template <typename Serializer>
class SerializerArray : public SerializerNode<Serializer> {
    template <typename S>
    friend class SerializerArray;
public:
    using Node = SerializerNode<Serializer>;

    SerializerArray(Node& parent) noexcept
        : Node(parent, true)
    {}
    template <std::derived_from<Serializer> OtherSerializer>
    SerializerArray(SerializerArray<OtherSerializer>& other) noexcept
        : Node(other, false)
    {}
    SerializerArray(SerializerArray&& other) noexcept
        : Node(std::move(other))
    {}
    template <std::derived_from<Serializer> OtherSerializer>
    SerializerArray(SerializerArray<OtherSerializer>&& other) noexcept
        : Node(std::move(other))
    {}

    ~SerializerArray() {
        if (this->m_ownsClose) {
            HUSE_ASSERT_INTERNAL(this->m_serializer);
            this->m_serializer->closeArray();
        }
    }
};

template <typename Serializer>
class SerializerObject : private SerializerNode<Serializer> {
    template <typename S>
    friend class SerializerObject;
public:
    using Node = SerializerNode<Serializer>;

    SerializerObject(Node& parent) noexcept
        : Node(parent, true)
    {}
    template <std::derived_from<Serializer> OtherSerializer>
    SerializerObject(SerializerObject<OtherSerializer>& other) noexcept
        : Node(other, false)
    {}
    SerializerObject(SerializerObject&& other) noexcept
        : Node(std::move(other))
    {}
    template <std::derived_from<Serializer> OtherSerializer>
    SerializerObject(SerializerObject<OtherSerializer>&& other) noexcept
        : Node(std::move(other))
    {}

    ~SerializerObject() {
        if (this->m_ownsClose) {
            HUSE_ASSERT_INTERNAL(this->m_serializer);
            this->m_serializer->closeObject();
        }
    }

    using Node::throwException;
    using Node::_active;
    using Node::_s;

    Node& key(std::string_view k) {
        this->m_serializer->pushKey(k);
        return *this;
    }
    Node& key(std::initializer_list<std::string_view> kp) {
        this->m_serializer->pushKeyParts(kp);
        return *this;
    }

    SerializerObject obj(std::string_view k) {
        return key(k).obj();
    }
    SerializerArray<Serializer> ar(std::string_view k) {
        return key(k).ar();
    }

    template <typename K, typename V>
    void val(K&& k, V&& v) {
        key(std::forward<K>(k)).val(std::forward<V>(v));
    }

    template <typename K, typename V, typename F>
    void cval(K&& k, V&& v, F&& f) {
        key(std::forward<K>(k)).cval(std::forward<V>(v), std::forward<F>(f));
    }

    template <typename FV>
    void flatval(FV&& v);

    /////////////////
    // compatibility with deserializer
    template <typename K, typename V>
    bool optval(K&& k, V&& v) {
        val(std::forward<K>(k), std::forward<V>(v));
        return true;
    }
    template <typename V>
    void keyval(std::string_view k, V&& v) {
        val(k, std::forward<V>(v));
    }
    template <typename V>
    bool optkeyval(std::string_view k, V&& v) {
        val(k, std::forward<V>(v));
        return true;
    }
    /////////////////
};

template <typename Serializer>
SerializerArray<Serializer> SerializerNode<Serializer>::ar() {
    return open(Array{});
}
template <typename Serializer>
SerializerObject<Serializer> SerializerNode<Serializer>::obj() {
    return open(Object{});
}

namespace impl {

template <typename Serializer, typename T>
concept HasWriteValue = requires(Serializer& s, T t) {
    s.writeValue(t);
};
template <typename T, typename Serializer>
concept HasSerializeMethod = requires(T t, SerializerNode<Serializer>& node) {
    t.huseSerialize(node);
};
template <typename T, typename Serializer>
concept HasSerializeFunc = requires(SerializerNode<Serializer>& node, T t) {
    huseSerialize(node, t);
};

template <typename Serializer, typename T>
concept HasOpen = requires(Serializer & s, T t) {
    s.open(t);
};
template <typename T, typename Serializer>
concept HasOpenMethod = requires(T t, SerializerNode<Serializer>&node) {
    t.huseOpen(node);
};
template <typename T, typename Serializer>
concept HasOpenFunc = requires(SerializerNode<Serializer>&node, T t) {
    huseOpen(node, t);
};


template <typename T, typename Serializer>
concept HasSerializeFlatMethod = requires(T t, SerializerObject<Serializer>& obj) {
    t.huseSerializeFlat(obj);
};
template <typename T, typename Serializer>
concept HasSerializeFlatFunc = requires(SerializerObject<Serializer>& obj, T t) {
    huseSerializeFlat(obj, t);
};
}

template <typename Serializer>
template <typename V>
void SerializerNode<Serializer>::val(V&& v) {
    if constexpr (impl::HasWriteValue<Serializer, V>) {
        m_serializer->writeValue(std::forward<V>(v));
    }
    else if constexpr (impl::HasSerializeMethod<V, Serializer>) {
        std::forward<V>(v).huseSerialize(*this);
    }
    else if constexpr (impl::HasSerializeFunc<V, Serializer>) {
        huseSerialize(*this, std::forward<V>(v));
    }
    else {
        huseCannotSerialize(*this, v);
    }
}

template <typename Serializer>
template <typename O>
decltype(auto) SerializerNode<Serializer>::open(O&& o) {
    if constexpr (impl::HasOpen<Serializer, O>) {
        m_serializer->open(std::forward<O>(o));
        return typename O::template SerializerNode<Serializer>(*this);
    }
    else if constexpr (impl::HasOpenMethod<O, Serializer>) {
        return std::forward<O>(o).huseOpen(*this);
    }
    else if constexpr (impl::HasOpenFunc<O, Serializer>) {
        return huseOpen(*this, std::forward<O>(o));
    }
    else {
        return huseCannotOpen(*this, o);
    }
}

template <typename Serializer>
template <typename FV>
void SerializerObject<Serializer>::flatval(FV&& v) {
    if constexpr (impl::HasSerializeFlatMethod<FV, Serializer>) {
        std::forward<FV>(v).huseSerializeFlat(*this);
    }
    else if constexpr (impl::HasSerializeFlatFunc<FV, Serializer>) {
        huseSerializeFlat(*this, std::forward<FV>(v));
    }
    else {
        huseCannotSerializeFlat(*this, v);
    }
}

} // namespace huse

