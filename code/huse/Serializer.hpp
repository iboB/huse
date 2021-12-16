// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once
#include "API.h"

#include "impl/UniqueStack.hpp"

#include <string_view>
#include <optional>
#include <cstdint>
#include <iosfwd>

namespace huse
{

class SerializerArray;
class SerializerObject;
class BasicSerializer;

class SerializerSStream : public impl::UniqueStack
{
public:
    SerializerSStream(BasicSerializer& s, impl::UniqueStack* parent);
    ~SerializerSStream();

    SerializerSStream(const SerializerSStream&) = delete;
    SerializerSStream& operator=(const SerializerSStream&) = delete;
    SerializerSStream(SerializerSStream&& other) noexcept = delete;
    SerializerSStream& operator=(SerializerSStream&&) = delete;

    template <typename T>
    SerializerSStream& operator<<(const T& t)
    {
        m_stream << t;
        return *this;
    }

    template <typename T>
    SerializerSStream& operator&(const T& t)
    {
        m_stream << t;
        return *this;
    }

    std::ostream& get() { return m_stream; }

private:
    BasicSerializer& m_serializer;
    std::ostream& m_stream;
};

class SerializerNode : public impl::UniqueStack
{
protected:
    SerializerNode(BasicSerializer& s, impl::UniqueStack* parent)
        : impl::UniqueStack(parent)
        , m_serializer(s)
    {}

    BasicSerializer& m_serializer;
public:
    SerializerNode(const SerializerNode&) = delete;
    SerializerNode& operator=(const SerializerNode&) = delete;
    SerializerNode(SerializerNode&&) = delete;
    SerializerNode& operator=(SerializerNode&&) = delete;

    uintptr_t context() const;

    SerializerObject obj();
    SerializerArray ar();

    template <typename T>
    void val(const T& v);

    template <typename T, typename F>
    void cval(const T& v, F f)
    {
        f(*this, v);
    }

    SerializerSStream sstream()
    {
        return SerializerSStream(m_serializer, this);
    }
};

class SerializerArray : public SerializerNode
{
public:
    SerializerArray(BasicSerializer& s, impl::UniqueStack* parent = nullptr);
    ~SerializerArray();
};

class SerializerObject : private SerializerNode
{
public:
    SerializerObject(BasicSerializer& s, impl::UniqueStack* parent = nullptr);
    ~SerializerObject();

    SerializerNode& key(std::string_view k);

    SerializerObject obj(std::string_view k)
    {
        return key(k).obj();
    }
    SerializerArray ar(std::string_view k)
    {
        return key(k).ar();
    }

    template <typename T>
    void val(std::string_view k, const T& v)
    {
        key(k).val(v);
    }

    template <typename T>
    void val(std::string_view k, const std::optional<T>& v)
    {
        if (v) val(k, *v);
    }

    template <typename T>
    void optval(std::string_view k, const T& v) { val(k, v); } // compatibility with deserializer

    template <typename T>
    void optval(std::string_view k, const std::optional<T>& v, const T& d)
    {
        if (v) val(k, *v);
        else val(k, d);
    }

    template <typename T>
    void flatval(const T& v);

    template <typename T, typename F>
    void cval(std::string_view k, const T& v, F f)
    {
        key(k).cval(v, std::move(f));
    }

    template <typename T, typename F>
    void cval(std::string_view k, const std::optional<T>& v, F f)
    {
        if (v) cval(k, *v, std::move(f));
    }

    SerializerSStream sstream(std::string_view k)
    {
        return key(k).sstream();
    }
};

class HUSE_API BasicSerializer : public SerializerNode
{
    friend class SerializerSStream;
    friend class SerializerNode;
    friend class SerializerArray;
    friend class SerializerObject;
public:
    BasicSerializer(uintptr_t ctx) : SerializerNode(*this, nullptr), m_context(ctx) {}
    virtual ~BasicSerializer();

    [[noreturn]] virtual void throwException(const std::string& msg) const;

protected:
    // write interface
    virtual void write(bool val) = 0;
    virtual void write(short val) = 0;
    virtual void write(unsigned short val) = 0;
    virtual void write(int val) = 0;
    virtual void write(unsigned int val) = 0;
    virtual void write(long val) = 0;
    virtual void write(unsigned long val) = 0;
    virtual void write(long long val) = 0;
    virtual void write(unsigned long long val) = 0;
    virtual void write(float val) = 0;
    virtual void write(double val) = 0;
    virtual void write(std::string_view val) = 0;

    // explicit calls
    virtual void write(nullptr_t) = 0; // write null explicitly
    virtual void write(std::nullopt_t) = 0; // discard current value

    // stateful writes
    virtual std::ostream& openStringStream() = 0;
    virtual void closeStringStream() = 0;

    // helpers
    void write(const char* s) { write(std::string_view(s)); }

    // implementation interface
    virtual void pushKey(std::string_view k) = 0;

    virtual void openObject() = 0;
    virtual void closeObject() = 0;

    virtual void openArray() = 0;
    virtual void closeArray() = 0;

private:
    uintptr_t m_context;
};

inline SerializerSStream::SerializerSStream(BasicSerializer& s, impl::UniqueStack* parent)
    : impl::UniqueStack(parent)
    , m_serializer(s)
    , m_stream(s.openStringStream())
{}

inline SerializerSStream::~SerializerSStream()
{
    m_serializer.closeStringStream();
}

inline uintptr_t SerializerNode::context() const
{
    return m_serializer.m_context;
}

inline SerializerObject SerializerNode::obj()
{
    return SerializerObject(m_serializer, this);
}

inline SerializerArray SerializerNode::ar()
{
    return SerializerArray(m_serializer, this);
}

namespace impl
{
struct SerializerWriteHelper : public BasicSerializer {
    using BasicSerializer::write;
};

template <typename, typename = void>
struct HasSerializerWrite : std::false_type {};

template <typename T>
struct HasSerializerWrite<T, decltype(std::declval<SerializerWriteHelper>().write(std::declval<T>()))> : std::true_type {};

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
void SerializerNode::val(const T& v)
{
    if constexpr (impl::HasSerializeMethod<T>::value)
    {
        v.huseSerialize(*this);
    }
    else if constexpr (impl::HasSerializeFunc<T>::value)
    {
        huseSerialize(*this, v);
    }
    // check last to avoid situations where implicit cast is possible and serialize specializations exist for T
    else if constexpr (impl::HasSerializerWrite<T>::value)
    {
        m_serializer.write(v);
    }
    else
    {
        cannot_serialize(v);
    }
}

inline SerializerArray::SerializerArray(BasicSerializer& s, impl::UniqueStack* parent)
    : SerializerNode(s, parent)
{
    m_serializer.openArray();
}

inline SerializerArray::~SerializerArray()
{
    m_serializer.closeArray();
}

inline SerializerObject::SerializerObject(BasicSerializer& s, impl::UniqueStack* parent)
    : SerializerNode(s, parent)
{
    m_serializer.openObject();
}

inline SerializerObject::~SerializerObject()
{
    m_serializer.closeObject();
}

inline SerializerNode& SerializerObject::key(std::string_view k)
{
    m_serializer.pushKey(k);
    return *this;
}

template <typename T>
void SerializerObject::flatval(const T& v)
{
    if constexpr (impl::HasSerializeFlatMethod<T>::value)
    {
        v.huseSerializeFlat(*this);
    }
    else if constexpr (impl::HasSerializeFlatFunc<T>::value)
    {
        huseSerializeFlat(*this, v);
    }
    else
    {
        cannot_serialize(v);
    }
}

} // namespace huse
