// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "ImValue.hpp"

#include <itlib/mem_streambuf.hpp>
#include <splat/unreachable.h>
#include <string_view>
#include <optional>
#include <istream>

namespace huse {

class DeserializerSStream {
public:
    explicit DeserializerSStream(std::string_view str)
        : m_streambuf(str.data(), str.size())
        , m_stream(&m_streambuf)
    {}

    DeserializerSStream(const DeserializerSStream&) = delete;
    DeserializerSStream& operator=(const DeserializerSStream&) = delete;

    template <typename T>
    DeserializerSStream& operator>>(T& t) {
        m_stream >> t;
        return *this;
    }

    template <typename T>
    DeserializerSStream& operator&(T& t) {
        m_stream >> t;
        return *this;
    }

    std::istream& get() { return m_stream; }

private:
    itlib::mem_istreambuf<char> m_streambuf;
    std::istream m_stream;
};

namespace impl {
struct DeserializerNodeImpl {
    // have template independent functions here to reduce code bloat and speed up compile times
protected:
    ImValue m_value;

    DeserializerNodeImpl(const ImValue& value)
        : m_value(value)
    {}

    // number of elements in compound object
    int size() const {
        return int(m_value.get_length());
    }

public:
    Type type() const {
        return m_value.htype();
    }

    [[noreturn]] void throwException(std::string_view msg) const {
        m_value.throwException(msg);
        SPLAT_UNREACHABLE();
    }
};
}

template <typename Deserializer>
class DeserializerObject;
template <typename Deserializer>
class DeserializerArray;

template <typename Deserializer>
class DeserializerNode : public impl::DeserializerNodeImpl {
protected:
    template <typename D>
    friend class DeserializerNode;

    Deserializer* m_deserializer;

public:
    DeserializerNode(const ImValue& value, Deserializer* d = nullptr) noexcept
        : impl::DeserializerNodeImpl(value)
        , m_deserializer(d)
    {}

    DeserializerNode(const DeserializerNode& other) = default;

    template <std::derived_from<Deserializer> OtherDeserializer>
    DeserializerNode(const DeserializerNode<OtherDeserializer>& other) noexcept
        : impl::DeserializerNodeImpl(other.m_value)
        , m_deserializer(other.m_deserializer)
    {}

    Deserializer& _d() noexcept { return *m_deserializer; }

    template <typename T>
    void val(T& v);

    template <typename T, typename F>
    void cval(T& v, F&& f) {
        f(*this, v);
    }

    template <typename O>
    decltype(auto) open(O&& o);

    DeserializerArray<Deserializer> ar();
    DeserializerObject<Deserializer> obj();

    DeserializerSStream sstream() {
        std::string_view str;
        val(str);
        return DeserializerSStream(str);
    }
};

template <typename Deserializer>
class DeserializerArray : public DeserializerNode<Deserializer> {
    template <typename D>
    friend class DeserializerArray;

    int m_index = 0;
public:
    using Node = DeserializerNode<Deserializer>;

    explicit DeserializerArray(const ImValue& value, Deserializer* d = nullptr)
        : Node(value, d)
    {
        if (!value.htype().isArray()) {
            this->throwException("not an array");
        }
    }

    DeserializerArray(const DeserializerArray& other) = default;

    template <std::derived_from<Deserializer> OtherDeserializer>
    DeserializerArray(const DeserializerArray<OtherDeserializer>& other) noexcept
        : Node(other)
        , m_index(other.m_index)
    {}

    using Node::size;

    DeserializerObject<Deserializer> obj();
    DeserializerArray<Deserializer> ar();

    std::optional<Node> optval() {
        if (done()) {
            return std::nullopt;
        }
        auto ret = Node(this->m_value.get_array_element(m_index), this->m_deserializer);
        ++m_index;
        return ret;
    }

    Node val() {
        auto v = optval();
        if (!v) {
            this->throwException("array index out of bounds");
        }
        return *v;
    }

    Node index(int index) {
        m_index = index;
        return val();
    }

    template <typename T>
    void val(T& v) {
        val().val(v);
    }

    template <typename T, typename F>
    void cval(T& v, F&& f) {
        f(val(), v);
    }

    DeserializerSStream sstream() {
        return val().sstream();
    }

    void skip() {
        ++m_index;
    }
    bool done() const {
        return m_index >= size();
    }

    // intentionally hiding parent
    Type type() const { return {Type::Array}; }

    class iterator {
        int m_index = 0;
        const DeserializerArray* m_array = nullptr;
    public:
        iterator() = default;
        explicit iterator(const DeserializerArray& a, int index)
            : m_index(index)
            , m_array(&a)
        {}

        Node operator*() const {
            return Node(m_array->m_value.get_array_element(m_index));
        }

        iterator& operator++() {
            ++m_index;
            return *this;
        }

        bool operator==(const iterator& other) const = default;
        bool operator!=(const iterator& other) const = default;
    };
    iterator begin() const {
        return iterator(*this, 0);
    }
    iterator end() const {
        return iterator(*this, size());
    }
};

template <typename Deserializer>
class DeserializerObject : public DeserializerNode<Deserializer> {
    template <typename D>
    friend class DeserializerObject;

    int m_index = 0;
public:
    using Node = DeserializerNode<Deserializer>;

    explicit DeserializerObject(const ImValue& value, Deserializer* d = nullptr)
        : Node(value, d)
    {
        if (!value.htype().isObject()) {
            this->throwException("not an object");
        }
    }

    DeserializerObject(const DeserializerObject& other) = default;

    template <std::derived_from<Deserializer> OtherDeserializer>
    DeserializerObject(const DeserializerObject<OtherDeserializer>& other) noexcept
        : Node(other)
        , m_index(other.m_index)
    {}

    using Node::size;

    void skip() {
        ++m_index;
    }
    bool done() const {
        return m_index >= size();
    }

    std::optional<Node> optkey(std::string_view k) {
        auto index = /*iile*/[&]() {
            if (!done() && this->m_value.get_object_key(m_index) == k) {
                return m_index;
            }
            return int(this->m_value.find_object_key(k));
        }();
        if (index >= size()) {
            m_index = index;
            return std::nullopt;
        }
        m_index = index + 1;
        return Node(this->m_value.get_object_value(index));
    }

    Node key(std::string_view k) {
        auto ret = optkey(k);
        if (!ret) {
            this->throwException("key not found in object");
        }
        return *ret;
    }

    DeserializerObject<Deserializer> obj(std::string_view k) {
        return key(k).obj();
    }
    DeserializerArray<Deserializer> ar(std::string_view k) {
        return key(k).ar();
    }

    template <typename T>
    void val(std::string_view k, T& v) {
        key(k).val(v);
    }

    template <typename T>
    void val(std::string_view k, std::optional<T>& v) {
        if (auto open = optkey(k)) {
            open->val(v.emplace());
        }
        else {
            v.reset();
        }
    }

    template <typename T>
    bool optval(std::string_view k, T& v) {
        if (auto open = optkey(k)) {
            open->val(v);
            return true;
        }
        return false;
    }

    template <typename T>
    bool optval(std::string_view k, std::optional<T>& v, const T& d) {
        if (auto open = optkey(k)) {
            open->val(v.emplace());
            return true;
        }
        else {
            v.reset(d);
            return false;
        }
    }

    template <typename T>
    void flatval(T& v);

    template <typename T, typename F>
    void cval(std::string_view k, T& v, F&& f) {
        key(k).cval(v, std::forward<F>(f));
    }

    template <typename T, typename F>
    void cval(std::string_view k, std::optional<T>& v, F&& f) {
        if (auto open = optkey(k)) {
            open->cval(v.emplace(), std::forward<F>(f));
        }
        else {
            v.reset();
        }
    }

    DeserializerSStream sstream(std::string_view k) {
        return key(k).sstream();
    }

    std::optional<std::pair<std::string_view, Node>> optkeyval() {
        if (done()) {
            return std::nullopt;
        }
        auto key = this->m_value.get_object_key(m_index);
        auto val = Node(this->m_value.get_object_value(m_index));
        ++m_index;
        return std::make_pair(key, val);
    }

    std::pair<std::string_view, Node> keyval() {
        auto r = optkeyval();
        if (!r) {
            this->throwException("no more keys in object");
        }
        return *r;
    }

    template <typename Key, typename T>
    void keyval(Key& k, T& v) {
        auto p = keyval();
        k = Key(p.first);
        p.second.val(v);
    }

    template <typename Key, typename T>
    bool optkeyval(Key& k, T& v) {
        auto p = optkeyval();
        if (!p) return false;
        k = Key(p->first);
        p->second.val(v);
        return true;
    }

    // intentionally hiding parent
    Type type() const { return {Type::Object}; }

    class iterator {
        int m_index = 0;
        const DeserializerObject* m_object = nullptr;
    public:
        iterator() = default;
        explicit iterator(const DeserializerObject& o, int index)
            : m_index(index)
            , m_object(&o)
        {}

        std::pair<std::string_view, Node> operator*() const {
            auto& val = m_object->m_value;
            return {
                val.get_object_key(m_index),
                Node(val.get_object_value(m_index))
            };
        }

        iterator& operator++() {
            ++m_index;
            return *this;
        }

        bool operator==(const iterator& other) const = default;
        bool operator!=(const iterator& other) const = default;
    };
    iterator begin() const {
        return iterator(*this, 0);
    }
    iterator end() const {
        return iterator(*this, size());
    }
};

template <typename Deserializer>
inline DeserializerObject<Deserializer> DeserializerNode<Deserializer>::obj() {
    return DeserializerObject<Deserializer>(m_value);
}
template <typename Deserializer>
inline DeserializerArray<Deserializer> DeserializerNode<Deserializer>::ar() {
    return DeserializerArray<Deserializer>(m_value);
}

template <typename Deserializer>
inline DeserializerObject<Deserializer> DeserializerArray<Deserializer>::obj() {
    return index(m_index).obj();
}
template <typename Deserializer>
inline DeserializerArray<Deserializer> DeserializerArray<Deserializer>::ar() {
    return index(m_index).ar();
}

namespace impl {

template <typename Deserializer, typename T>
concept HasDeserializerGetValue = requires(Deserializer& d, ImValue& v, T& t) {
    d.getValue(v, t);
};
template <typename T>
concept HasGetValue = requires(ImValue& v, T& t) {
    v.getValue(t);
};
template <typename T, typename Deserializer>
concept HasDeserializeMethod = requires(T& t, DeserializerNode<Deserializer>& node) {
    t.huseDeserialize(node);
};
template <typename T, typename Deserializer>
concept HasDeserializeFunc = requires(T& t, DeserializerNode<Deserializer>& node) {
    huseDeserialize(node, t);
};

template <typename Deserializer, typename T>
concept HasDOpen = requires(Deserializer& d, T t, ImValue& v) {
    d.open(t, v);
};
template <typename T, typename Deserializer>
concept HasOpenDMethod = requires(T t, DeserializerNode<Deserializer>& node) {
    t.huseOpen(node);
};
template <typename T, typename Deserializer>
concept HasOpenDFunc = requires(DeserializerNode<Deserializer>& node, T t) {
    huseOpen(node, t);
};

template <typename T, typename Deserializer>
concept HasDeserializeFlatMethod = requires(T t, DeserializerObject<Deserializer>& obj) {
    t.huseDeserializeFlat(obj);
};
template <typename T, typename Deserializer>
concept HasDeserializeFlatFunc = requires(DeserializerObject<Deserializer>& obj, T& t) {
    huseDeserializeFlat(obj, t);
};
} // namespace impl

template <typename Deserializer>
template <typename T>
void DeserializerNode<Deserializer>::val(T& v) {
    if constexpr (impl::HasDeserializerGetValue<Deserializer, T>) {
        this->m_deserializer->getValue(m_value, v);
    }
    else if constexpr (impl::HasGetValue<T>) {
        m_value.getValue(v);
    }
    else if constexpr (impl::HasDeserializeMethod<T, Deserializer>) {
        v.huseDeserialize(*this);
    }
    else if constexpr (impl::HasDeserializeFunc<T, Deserializer>) {
        huseDeserialize(*this, v);
    }
    else {
        huseCannotDeserialize(v);
    }
}

template <typename Deserializer>
template <typename O>
decltype(auto) DeserializerNode<Deserializer>::open(O&& o) {
    if constexpr (impl::HasDOpen<Deserializer, O>) {
        return typename O::template DeserializerNode<Deserializer>(
            this->m_deserializer->open(std::forward<O>(o), m_value),
            this->m_deserializer
        );
    }
    else if constexpr (impl::HasOpenDMethod<O, Deserializer>) {
        return std::forward<O>(o).huseOpen(*this);
    }
    else if constexpr (impl::HasOpenDFunc<O, Deserializer>) {
        return huseOpen(*this, std::forward<O>(o));
    }
    else {
        return huseCannotOpen(*this, o);
    }
}

template <typename Deserializer>
template <typename T>
void DeserializerObject<Deserializer>::flatval(T& v) {
    if constexpr (impl::HasDeserializeFlatMethod<T, Deserializer>) {
        v.huseDeserializeFlat(*this);
    }
    else if constexpr (impl::HasDeserializeFlatFunc<T, Deserializer>) {
        huseDeserializeFlat(*this, v);
    }
    else {
        huseCannotDeserializeFlat(v);
    }
}

} // namespace huse
