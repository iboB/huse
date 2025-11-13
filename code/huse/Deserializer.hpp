// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "impl/RawDValue.hpp"
#include "DeserializerInterface.hpp"
#include "DeserializerObj.hpp"

#include <itlib/mem_streambuf.hpp>
#include <splat/unreachable.h>
#include <string_view>
#include <optional>
#include <istream>

namespace huse
{
class DeserializerArray;
class DeserializerObject;

class DeserializerSStream
{
public:
    explicit DeserializerSStream(Deserializer& d, std::string_view str)
        : m_deserializer(d)
        , m_streambuf(str.data(), str.size())
        , m_stream(&m_streambuf)
    {}

    DeserializerSStream(const DeserializerSStream&) = delete;
    DeserializerSStream& operator=(const DeserializerSStream&) = delete;

    template <typename T>
    DeserializerSStream& operator>>(T& t)
    {
        m_stream >> t;
        return *this;
    }

    template <typename T>
    DeserializerSStream& operator&(T& t)
    {
        m_stream >> t;
        return *this;
    }

    std::istream& get() { return m_stream; }

private:
    Deserializer& m_deserializer;

    itlib::mem_istreambuf<char> m_streambuf;
    std::istream m_stream;
};

class DeserializerNode
{
protected:
    friend class Deserializer;
    Deserializer& m_deserializer;
    impl::RawDValue m_value;
public:
    explicit DeserializerNode(Deserializer& d, const impl::RawDValue& value)
        : m_deserializer(d)
        , m_value(value)
    {}

    Deserializer& _s() { return m_deserializer; }

    Type type() const {
        return m_value.htype();
    }

    DeserializerObject obj();
    DeserializerArray ar();

    template <typename T>
    void val(T& v);

    template <typename T, typename F>
    void cval(T& v, F&& f)
    {
        f(*this, v);
    }

    DeserializerSStream sstream()
    {
        std::string_view str;
        val(str);
        return DeserializerSStream(m_deserializer, str);
    }

    [[noreturn]] void throwException(std::string_view msg) const;

protected:
    // number of elements in compound object
    int length() const {
        return int(m_value.get_length());
    }
};

class DeserializerArray : public DeserializerNode
{
    int m_index = 0;
public:
    explicit DeserializerArray(Deserializer& d, const impl::RawDValue& value)
        : DeserializerNode(d, value)
    {
        if (!value.htype().isArray()) {
            throwException("not an array");
        }
    }

    using DeserializerNode::length;
    DeserializerNode index(int index) {
        if (index < 0 || index >= length()) {
            throwException("array index out of bounds");
        }
        m_index = index + 1;
        return DeserializerNode(m_deserializer, m_value.get_array_element(index));
    }

    DeserializerObject obj();
    DeserializerArray ar();

    template <typename T>
    void val(T& v) {
        index(m_index).val(v);
    }

    template <typename T, typename F>
    void cval(T& v, F&& f) {
        f(index(m_index), v);
    }

    DeserializerSStream sstream() {
        return index(m_index).sstream();
    }

    void skip() {
        ++m_index;
    }
    bool end() const {
        return m_index >= length();
    }

    // intentionally hiding parent
    Type type() const { return { Type::Array }; }

    std::optional<DeserializerNode> peeknext() {
        if (end()) {
            return std::nullopt;
        }
        return DeserializerNode(m_deserializer, m_value.get_array_element(m_index));
    }
};

class DeserializerObject : public DeserializerNode
{
    int m_index = 0;
public:
    explicit DeserializerObject(Deserializer& d, const impl::RawDValue& value)
        : DeserializerNode(d, value)
    {
        if (!value.htype().isObject()) {
            throwException("not an object");
        }
    }

    using DeserializerNode::_s;
    using DeserializerNode::length;
    using DeserializerNode::throwException;

    void skip() {
        ++m_index;
    }
    bool end() const {
        return m_index >= length();
    }

    DeserializerNode key(std::string_view k) {
        auto index = int(m_value.find_object_key(k));
        if (index >= length()) {
            throwException("key not found: " + std::string(k));
        }
        m_index = index + 1;
        return DeserializerNode(m_deserializer, m_value.get_object_value(index));
    }

    DeserializerObject obj(std::string_view k) {
        return key(k).obj();
    }
    DeserializerArray ar(std::string_view k) {
        return key(k).ar();
    }

    std::optional<DeserializerNode> optkey(std::string_view k) {
        auto index = int(m_value.find_object_key(k));
        if (index >= length()) {
            return std::nullopt;
        }
        m_index = index + 1;
        return DeserializerNode(m_deserializer, m_value.get_object_value(index));
    }

    template <typename T>
    void val(std::string_view k, T& v)
    {
        key(k).val(v);
    }

    template <typename T>
    void val(std::string_view k, std::optional<T>& v)
    {
        if (auto open = optkey(k))
        {
            open->val(v.emplace());
        }
        else
        {
            v.reset();
        }
    }

    template <typename T>
    bool optval(std::string_view k, T& v)
    {
        if (auto open = optkey(k))
        {
            open->val(v);
            return true;
        }
        return false;
    }

    template <typename T>
    bool optval(std::string_view k, std::optional<T>& v, const T& d)
    {
        if (auto open = optkey(k))
        {
            open->val(v.emplace());
            return true;
        }
        else
        {
            v.reset(d);
            return false;
        }
    }

    template <typename T>
    void flatval(T& v);

    template <typename T, typename F>
    void cval(std::string_view k, T& v, F&& f)
    {
        key(k).cval(v, std::forward<F>(f));
    }

    template <typename T, typename F>
    void cval(std::string_view k, std::optional<T>& v, F&& f)
    {
        if (auto open = optkey(k))
        {
            open->cval(v.emplace(), std::forward<F>(f));
        }
        else
        {
            v.reset();
        }
    }

    DeserializerSStream sstream(std::string_view k)
    {
        return key(k).sstream();
    }

    struct KeyQuery
    {
        std::string_view name;
        std::optional<DeserializerNode> node;
        explicit operator bool() const { return !!node; }
        DeserializerNode* operator->() { return &(*node); }
    };
    KeyQuery peeknext() {
        if (end()) {
            return {};
        }
        KeyQuery ret;
        ret.name = m_value.get_object_key(m_index);
        ret.node.emplace(m_deserializer, m_value.get_object_value(m_index));

        return ret;
    }

    template <typename Key, typename T>
    void keyval(Key& k, T& v) {
        auto p = peeknext();
        if (!p) {
            throwException("no more keys in object");
        }
        ++m_index;
        k = Key(p.name);
        p->val(v);
    }

    // intentionally hiding parent
    Type type() const { return { Type::Object }; }
};

inline DeserializerObject DeserializerNode::obj()
{
    return DeserializerObject(m_deserializer, m_value);
}

inline DeserializerArray DeserializerNode::ar()
{
    return DeserializerArray(m_deserializer, m_value);
}

inline DeserializerObject DeserializerArray::obj() {
    return index(m_index).obj();
}
inline DeserializerArray DeserializerArray::ar() {
    return index(m_index).ar();
}

inline void DeserializerNode::throwException(std::string_view msg) const {
    m_value.throwException(msg);
    SPLAT_UNREACHABLE();
}

namespace impl
{
//template <typename, typename = void>
//struct HasPolyDeserialize : std::false_type {};
//template <typename T>
//struct HasPolyDeserialize<T, decltype(husePolyDeserialize(std::declval<Deserializer&>(), std::declval<T&>()))> : std::true_type {};

template <typename, typename = void>
struct HasGetValue : std::false_type {};
template <typename T>
struct HasGetValue<T, decltype(std::declval<impl::RawDValue>().getValue(std::declval<T&>()))> : std::true_type {};

template <typename, typename = void>
struct HasDeserializeMethod : std::false_type {};
template <typename T>
struct HasDeserializeMethod<T, decltype(std::declval<T>().huseDeserialize(std::declval<DeserializerNode&>()))> : std::true_type {};
template <typename, typename = void>
struct HasDeserializeFunc : std::false_type {};
template <typename T>
struct HasDeserializeFunc<T, decltype(huseDeserialize(std::declval<DeserializerNode&>(), std::declval<T&>()))> : std::true_type {};

template <typename, typename = void>
struct HasDeserializeFlatMethod : std::false_type {};
template <typename T>
struct HasDeserializeFlatMethod<T, decltype(std::declval<T>().huseDeserializeFlat(std::declval<DeserializerObject&>()))> : std::true_type {};
template <typename, typename = void>
struct HasDeserializeFlatFunc : std::false_type {};
template <typename T>
struct HasDeserializeFlatFunc<T, decltype(huseDeserializeFlat(std::declval<DeserializerObject&>(), std::declval<T&>()))> : std::true_type {};
} // namespace impl

template <typename T>
void DeserializerNode::val(T& v) {
    if constexpr (impl::HasGetValue<T>::value) {
        m_value.getValue(v);
    }
    else if constexpr (impl::HasDeserializeMethod<T>::value) {
        v.huseDeserialize(*this);
    }
    else if constexpr (impl::HasDeserializeFunc<T>::value) {
        huseDeserialize(*this, v);
    }
    //else if constexpr (impl::HasPolyDeserialize<T>::value)
    //{
    //    husePolyDeserialize(m_deserializer, v);
    //}
    else {
        cannot_deserialize(v);
    }
}

template <typename T>
void DeserializerObject::flatval(T& v)
{
    if constexpr (impl::HasDeserializeFlatMethod<T>::value)
    {
        v.huseDeserializeFlat(*this);
    }
    else if constexpr (impl::HasDeserializeFlatFunc<T>::value)
    {
        huseDeserializeFlat(*this, v);
    }
    else
    {
        cannot_deserialize(v);
    }
}

class DeserializerRoot : public DeserializerNode {
    Deserializer m_deserializerObject;
public:
    explicit DeserializerRoot(Deserializer&& d)
        : DeserializerNode(m_deserializerObject, root_msg::call(d))
        , m_deserializerObject(std::move(d))
    {}
};

inline DeserializerNode Deserializer::node() {
    return DeserializerNode(*this, root_msg::call(*this));
}

} // namespace huse
