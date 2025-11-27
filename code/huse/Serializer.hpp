// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "impl/UniqueStack.hpp"
#include "StatefulSerializer.hpp"

#include <string_view>
#include <string>
#include <optional>
#include <iosfwd>

namespace huse {
class SerializerNode;
class SerializerArray;
class SerializerObject;

class SerializerSStream : public impl::UniqueStack {
public:
    explicit SerializerSStream(StatefulSerializer& s, impl::UniqueStack* parent);
    ~SerializerSStream();

    SerializerSStream(const SerializerSStream&) = delete;
    SerializerSStream& operator=(const SerializerSStream&) = delete;
    SerializerSStream(SerializerSStream&& other) noexcept = delete;
    SerializerSStream& operator=(SerializerSStream&&) = delete;

    template <typename T>
    SerializerSStream& operator<<(const T& t) {
        m_stream << t;
        return *this;
    }

    template <typename T>
    SerializerSStream& operator&(const T& t) {
        m_stream << t;
        return *this;
    }

    std::ostream& get() { return m_stream; }

    [[noreturn]] void throwException(const std::string& msg) const;

private:
    StatefulSerializer& m_serializer;
    std::ostream& m_stream;
};

class SerializerNode : public impl::UniqueStack {
protected:
    StatefulSerializer& m_serializer;
public:
    explicit SerializerNode(StatefulSerializer& s, impl::UniqueStack* parent)
        : impl::UniqueStack(parent)
        , m_serializer(s)
    {}
    SerializerNode(const SerializerNode&) = delete;
    SerializerNode& operator=(const SerializerNode&) = delete;
    SerializerNode(SerializerNode&&) = delete;
    SerializerNode& operator=(SerializerNode&&) = delete;

    StatefulSerializer& _s() { return m_serializer; }

    SerializerObject obj();
    SerializerArray ar();

    template <typename T>
    void val(const T& v);

    template <typename T, typename F>
    void cval(const T& v, F&& f) {
        f(*this, v);
    }

    SerializerSStream sstream() {
        return SerializerSStream(m_serializer, this);
    }

    [[noreturn]] void throwException(const std::string& msg) const;
};

class SerializerArray : public SerializerNode {
public:
    explicit SerializerArray(StatefulSerializer& s, impl::UniqueStack* parent = nullptr);
    ~SerializerArray();
};

class SerializerObject : private SerializerNode {
public:
    explicit SerializerObject(StatefulSerializer& s, impl::UniqueStack* parent = nullptr);
    ~SerializerObject();

    using SerializerNode::_s;
    using SerializerNode::throwException;

    SerializerNode& key(std::string_view k);

    SerializerObject obj(std::string_view k) {
        return key(k).obj();
    }
    SerializerArray ar(std::string_view k) {
        return key(k).ar();
    }

    template <typename T>
    void val(std::string_view k, const T& v) {
        key(k).val(v);
    }

    template <typename T>
    void val(std::string_view k, const std::optional<T>& v) {
        if (v) val(k, *v);
    }

    template <typename T>
    bool optval(std::string_view k, const std::optional<T>& v, const T& d) {
        if (v) {
            val(k, *v);
            return true;
        }
        else {
            val(k, d);
            return false;
        }
    }

    template <typename T>
    void flatval(const T& v);

    template <typename T, typename F>
    void cval(std::string_view k, const T& v, F&& f) {
        key(k).cval(v, std::forward<F>(f));
    }

    template <typename T, typename F>
    void cval(std::string_view k, const std::optional<T>& v, F&& f) {
        if (v) cval(k, *v, std::forward<F>(f));
    }

    SerializerSStream sstream(std::string_view k) {
        return key(k).sstream();
    }

    /////////////////
    // compatibility with deserializer
    template <typename T> bool optval(std::string_view k, const T& v) { val(k, v); return true; }
    template <typename T> void keyval(std::string_view k, const T& v) { val(k, v); }
    template <typename T> bool optkeyval(std::string_view k, const T& v) { val(k, v); return true; }
    /////////////////
};

inline SerializerSStream::SerializerSStream(StatefulSerializer& s, impl::UniqueStack* parent)
    : impl::UniqueStack(parent)
    , m_serializer(s)
    , m_stream(s.openStringStream())
{}

inline SerializerSStream::~SerializerSStream() {
    m_serializer.closeStringStream();
}

inline void SerializerSStream::throwException(const std::string& msg) const {
    m_serializer.throwException(msg);
}

inline SerializerObject SerializerNode::obj() {
    return SerializerObject(m_serializer, this);
}

inline SerializerArray SerializerNode::ar() {
    return SerializerArray(m_serializer, this);
}

namespace impl
{
//template <typename, typename = void>
//struct HasPolySerialize : std::false_type {};
//template <typename T>
//struct HasPolySerialize<T, decltype(husePolySerialize(std::declval<StatefulSerializer&>(), std::declval<T>()))> : std::true_type {};

template <typename, typename = void>
struct HasWriteValue : std::false_type {};
template <typename T>
struct HasWriteValue<T, decltype(std::declval<StatefulSerializer&>().writeValue(std::declval<T>()))> : std::true_type {};

template <typename, typename = void>
struct HasSerializeMethod : std::false_type {};
template <typename T>
struct HasSerializeMethod<T, decltype(std::declval<T>().huseSerialize(std::declval<SerializerNode&>()))> : std::true_type {};
template <typename, typename = void>
struct HasSerializeFunc : std::false_type {};
template <typename T>
struct HasSerializeFunc<T, decltype(huseSerialize(std::declval<SerializerNode&>(), std::declval<T>()))> : std::true_type {};

template <typename, typename = void>
struct HasSerializeFlatMethod : std::false_type {};
template <typename T>
struct HasSerializeFlatMethod<T, decltype(std::declval<T>().huseSerializeFlat(std::declval<SerializerObject&>()))> : std::true_type {};
template <typename, typename = void>
struct HasSerializeFlatFunc : std::false_type {};
template <typename T>
struct HasSerializeFlatFunc<T, decltype(huseSerializeFlat(std::declval<SerializerObject&>(), std::declval<T>()))> : std::true_type {};
} // namespace impl

template <typename T>
void SerializerNode::val(const T& v) {
    if constexpr (impl::HasWriteValue<T>::value) {
        m_serializer.writeValue(v);
    }
    else if constexpr (impl::HasSerializeMethod<T>::value) {
        v.huseSerialize(*this);
    }
    else if constexpr (impl::HasSerializeFunc<T>::value) {
        huseSerialize(*this, v);
    }
    //else if constexpr (impl::HasPolySerialize<T>::value) {
    //    husePolySerialize(m_serializer, v);
    //}
    else {
        cannot_serialize(v);
    }
}

inline void SerializerNode::throwException(const std::string& msg) const {
    m_serializer.throwException(msg);
}

inline SerializerArray::SerializerArray(StatefulSerializer& s, impl::UniqueStack* parent)
    : SerializerNode(s, parent)
{
    m_serializer.openArray();
}

inline SerializerArray::~SerializerArray() {
    m_serializer.closeArray();
}

inline SerializerObject::SerializerObject(StatefulSerializer& s, impl::UniqueStack* parent)
    : SerializerNode(s, parent)
{
    m_serializer.openObject();
}

inline SerializerObject::~SerializerObject() {
    m_serializer.closeObject();
}

inline SerializerNode& SerializerObject::key(std::string_view k) {
    m_serializer.pushKey(k);
    return *this;
}

template <typename T>
void SerializerObject::flatval(const T& v) {
    if constexpr (impl::HasSerializeFlatMethod<T>::value) {
        v.huseSerializeFlat(*this);
    }
    else if constexpr (impl::HasSerializeFlatFunc<T>::value) {
        huseSerializeFlat(*this, v);
    }
    else {
        cannot_serialize(v);
    }
}

} // namespace huse
