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
#include <memory>

namespace huse
{
class CtxObj;
class DeserializerArray;
class DeserializerObject;

class DeserializerSStream
{
public:
    explicit DeserializerSStream(CtxObj& d, std::string_view str)
        : m_ctx(d)
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
    CtxObj& m_ctx;

    itlib::mem_istreambuf<char> m_streambuf;
    std::istream m_stream;
};

class DeserializerNode {
protected:
    CtxObj& m_ctx;
    ImValue m_value;
public:
    explicit DeserializerNode(CtxObj& d, const ImValue& value)
        : m_ctx(d)
        , m_value(value)
    {}

    CtxObj& ctx() { return m_ctx; }

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
        return DeserializerSStream(m_ctx, str);
    }

    [[noreturn]] void throwException(std::string_view msg) const {
        m_value.throwException(msg);
        SPLAT_UNREACHABLE();
    }

protected:
    // number of elements in compound object
    int size() const {
        return int(m_value.get_length());
    }
};

class DeserializerValue : public DeserializerNode {
};

class DeserializerArray : public DeserializerNode {
    int m_index = 0;
public:
    explicit DeserializerArray(CtxObj& d, const ImValue& value)
        : DeserializerNode(d, value)
    {
        if (!value.htype().isArray()) {
            throwException("not an array");
        }
    }

    using DeserializerNode::size;

    DeserializerObject obj();
    DeserializerArray ar();

    std::optional<DeserializerNode> optval() {
        if (done()) {
            return std::nullopt;
        }
        auto ret = DeserializerNode(m_ctx, m_value.get_array_element(m_index));
        ++m_index;
        return ret;
    }

    DeserializerNode val() {
        auto v = optval();
        if (!v) {
            throwException("array index out of bounds");
        }
        return *v;
    }

    DeserializerNode index(int index) {
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
    Type type() const { return { Type::Array }; }

    class iterator {
        int m_index = 0;
        const DeserializerArray* m_array = nullptr;
    public:
        iterator() = default;
        explicit iterator(const DeserializerArray& a, int index)
            : m_index(index)
            , m_array(&a)
        {}

        DeserializerNode operator*() const {
            return DeserializerNode(m_array->m_ctx, m_array->m_value.get_array_element(m_index));
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

class DeserializerObject : public DeserializerNode
{
    int m_index = 0;
public:
    explicit DeserializerObject(CtxObj& d, const ImValue& value)
        : DeserializerNode(d, value)
    {
        if (!value.htype().isObject()) {
            throwException("not an object");
        }
    }

    using DeserializerNode::size;

    void skip() {
        ++m_index;
    }
    bool done() const {
        return m_index >= size();
    }

    std::optional<DeserializerNode> optkey(std::string_view k) {
        auto index = /*iile*/[&]() {
            if (!done() && m_value.get_object_key(m_index) == k) {
                return m_index;
            }
            return int(m_value.find_object_key(k));
        }();
        if (index >= size()) {
            m_index = index;
            return std::nullopt;
        }
        m_index = index + 1;
        return DeserializerNode(m_ctx, m_value.get_object_value(index));
    }

    DeserializerNode key(std::string_view k) {
        auto ret = optkey(k);
        if (!ret) {
            throwException("key not found in object");
        }
        return *ret;
    }

    DeserializerObject obj(std::string_view k) {
        return key(k).obj();
    }
    DeserializerArray ar(std::string_view k) {
        return key(k).ar();
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

    std::optional<std::pair<std::string_view, DeserializerNode>> optkeyval() {
        if (done()) {
            return std::nullopt;
        }
        auto key = m_value.get_object_key(m_index);
        auto val = DeserializerNode(m_ctx, m_value.get_object_value(m_index));
        ++m_index;
        return std::make_pair(key, val);
    }

    std::pair<std::string_view, DeserializerNode> keyval() {
        auto r = optkeyval();
        if (!r) {
            throwException("no more keys in object");
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
    Type type() const { return { Type::Object }; }

    class iterator {
        int m_index = 0;
        const DeserializerObject* m_object = nullptr;
    public:
        iterator() = default;
        explicit iterator(const DeserializerObject& o, int index)
            : m_index(index)
            , m_object(&o)
        {}

        std::pair<std::string_view, DeserializerNode> operator*() const {
            auto& val = m_object->m_value;
            return {
                val.get_object_key(m_index),
                DeserializerNode(m_object->m_ctx, val.get_object_value(m_index))
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

inline DeserializerObject DeserializerNode::obj() {
    return DeserializerObject(m_ctx, m_value);
}

inline DeserializerArray DeserializerNode::ar() {
    return DeserializerArray(m_ctx, m_value);
}

inline DeserializerObject DeserializerArray::obj() {
    return index(m_index).obj();
}
inline DeserializerArray DeserializerArray::ar() {
    return index(m_index).ar();
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
struct HasGetValue<T, decltype(std::declval<ImValue>().getValue(std::declval<T&>()))> : std::true_type {};

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
    //    husePolyDeserialize(m_ctx, v);
    //}
    else {
        cannot_deserialize(v);
    }
}

template <typename T>
void DeserializerObject::flatval(T& v) {
    if constexpr (impl::HasDeserializeFlatMethod<T>::value) {
        v.huseDeserializeFlat(*this);
    }
    else if constexpr (impl::HasDeserializeFlatFunc<T>::value) {
        huseDeserializeFlat(*this, v);
    }
    else {
        cannot_deserialize(v);
    }
}

} // namespace huse
