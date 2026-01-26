#pragma once
#include "OpenStringStream.hpp"
#include <iosfwd>
#include <concepts>
#include <string_view>
#include <initializer_list>

namespace huse {

template <typename Serializer>
class SerializerObject;
template <typename Serializer>
class SerializerArray;
template <typename Serializer>
class SerializerSStream;

template <typename Serializer>
class SerializerNode {
protected:
    template <typename S>
    friend class SerializerNode;

    Serializer* m_serializer;

    int m_parentId;
    const int m_id;

    bool ownsClose() const {
        return m_parentId >= 0;
    }
public:
    SerializerNode(Serializer& s) noexcept
        : m_serializer(&s)
        , m_parentId(s.curNodeId())
        , m_id(s.getNewNodeId())
    {}

    template <std::derived_from<Serializer> OtherSerializer>
    SerializerNode(SerializerNode<OtherSerializer>& other, bool ownsClose = false) noexcept
        : m_serializer(other.m_serializer)
        , m_parentId(ownsClose ? other.m_id : -1)
        , m_id(ownsClose ? m_serializer->getNewNodeId() : other.m_id)
    {}

    SerializerNode(SerializerNode&& other) noexcept
        : m_serializer(other.m_serializer)
        , m_parentId(other.m_parentId)
        , m_id(other.m_id)
    {
        other.m_serializer = nullptr;
        other.m_parentId = -1;
    }

    template <std::derived_from<Serializer> OtherSerializer>
    SerializerNode(SerializerNode<OtherSerializer>&& other) noexcept
        : m_serializer(other.m_serializer)
        , m_parentId(other.m_parentId)
        , m_id(other.m_id)
    {
        other.m_serializer = nullptr;
        other.m_parentId = -1;
    }

    SerializerNode& operator=(const SerializerNode&) = delete;
    SerializerNode& operator=(SerializerNode&&) = delete;

    ~SerializerNode() {
        if (ownsClose()) {
            HUSE_ASSERT_INTERNAL(m_serializer);
            m_serializer->releaseNodeId(m_id, m_parentId);
        }
    }

    [[noreturn]] void throwException(const std::string& msg) const {
        m_serializer->throwException(msg);
    }

    bool _active() const noexcept { return !!m_serializer; }
    Serializer& _s() noexcept { return *m_serializer; }

    template <typename V>
    void val(V&& v);

    template <typename O>
    decltype(auto) open(O&& o) {
        return huseOpen(std::forward<O>(o), *this);
    }

    template <typename V, typename F>
    void cval(V&& v, F&& f) {
        f(*this, std::forward<V>(v));
    }

    SerializerArray<Serializer> ar();
    SerializerObject<Serializer> obj();
};

template <typename Serializer>
class SerializerSStream : private SerializerNode<Serializer> {
    std::ostream* m_stream;
public:
    using Node = SerializerNode<Serializer>;

    SerializerSStream(Node& parent, std::ostream& stream) noexcept
        : Node(parent, true)
        , m_stream(&stream)
    {}
    template <std::derived_from<Serializer> OtherSerializer>
    SerializerSStream(SerializerSStream<OtherSerializer>& other) noexcept
        : Node(other, false)
        , m_stream(other.m_stream)
    {}
    SerializerSStream(SerializerSStream&& other) noexcept
        : Node(std::move(other))
        , m_stream(other.m_stream)
    {
        other.m_stream = nullptr;
    }
    template <std::derived_from<Serializer> OtherSerializer>
    SerializerSStream(SerializerSStream<OtherSerializer>&& other) noexcept
        : Node(std::move(other))
        , m_stream(other.m_stream)
    {
        other.m_stream = nullptr;
    }

    ~SerializerSStream() {
        if (this->ownsClose()) {
            HUSE_ASSERT_INTERNAL(this->m_serializer);
            this->m_serializer->closeStringStream();
        }
    }

    using Node::throwException;
    using Node::_active;
    using Node::_s;

    template <typename T>
    SerializerSStream& operator<<(const T& t) {
        *m_stream << t;
        return *this;
    }

    template <typename T>
    SerializerSStream& operator&(const T& t) {
        *m_stream << t;
        return *this;
    }

    std::ostream& get() { return *m_stream; }
};

template <typename Serializer>
class SerializerArray : public SerializerNode<Serializer> {
    template <typename S>
    friend class SerializerArray;
public:
    using Node = SerializerNode<Serializer>;

    SerializerArray(Node& parent, int) noexcept
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
        if (this->ownsClose()) {
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

    SerializerObject(Node& parent, int) noexcept
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
        if (this->ownsClose()) {
            HUSE_ASSERT_INTERNAL(this->m_serializer);
            this->m_serializer->closeObject();
        }
    }

    using Node::throwException;
    using Node::_active;
    using Node::_s;

    template <typename O>
    decltype(auto) open(O&& o) {
        return huseOpen(std::forward<O>(o), *this);
    }

    Node& key(std::string_view k) {
        this->m_serializer->pushKey(k);
        return *this;
    }
    //Node& key(std::initializer_list<std::string_view> kp) {
    //    this->m_serializer->pushKeyParts(kp);
    //    return *this;
    //}

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

    template <typename K, typename OV>
    void optval(K&& k, OV&& ov) {
        if (!ov) return;
        key(std::forward<K>(k)).val(*std::forward<OV>(ov));
    }

    template <typename K, typename V, typename F>
    void cval(K&& k, V&& v, F&& f) {
        key(std::forward<K>(k)).cval(std::forward<V>(v), std::forward<F>(f));
    }

    template <typename FV>
    void flatval(FV&& v);

    /////////////////
    // compatibility with deserializer
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
    m_serializer->openArray();
    return SerializerArray<Serializer>(*this, 0);
}
template <typename Serializer>
SerializerObject<Serializer> SerializerNode<Serializer>::obj() {
    m_serializer->openObject();
    return SerializerObject<Serializer>(*this, 0);
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

template <typename Serializer>
SerializerSStream<Serializer> huseOpen(StringStream, SerializerNode<Serializer>& parent) {
    return SerializerSStream<Serializer>(parent, parent._s().openStringStream());
}

} // namespace huse

