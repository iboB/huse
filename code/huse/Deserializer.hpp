// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "DeserializerInterface.hpp"
#include "DeserializerObj.hpp"

#include "impl/UniqueStack.hpp"

#include <string_view>
#include <optional>
#include <iosfwd>

namespace huse
{
class DeserializerArray;
class DeserializerObject;

class DeserializerSStream : public impl::UniqueStack
{
public:
    DeserializerSStream(Deserializer& d, impl::UniqueStack* parent);
    ~DeserializerSStream();

    DeserializerSStream(const DeserializerSStream&) = delete;
    DeserializerSStream& operator=(const DeserializerSStream&) = delete;

    // can't delete this too, as we need it to be inside std::optional
    DeserializerSStream(DeserializerSStream&& other) noexcept
        : impl::UniqueStack(std::move(other))
        , m_deserializer(other.m_deserializer)
        , m_stream(other.m_stream)
    {
        other.m_stream = nullptr;
    }
    DeserializerSStream& operator=(DeserializerSStream&&) = delete;

    template <typename T>
    DeserializerSStream& operator>>(T& t)
    {
        *m_stream >> t;
        return *this;
    }

    template <typename T>
    DeserializerSStream& operator&(T& t)
    {
        *m_stream >> t;
        return *this;
    }

    std::istream& get() { return *m_stream; }

    [[noreturn]] void throwException(const std::string& msg) const;

private:
    Deserializer& m_deserializer;
    std::istream* m_stream;
};

class DeserializerNode : public impl::UniqueStack
{
protected:
    DeserializerNode(Deserializer& d, impl::UniqueStack* parent)
        : impl::UniqueStack(parent)
        , m_deserializer(d)
    {}

    friend class Deserializer;
    Deserializer& m_deserializer;
public:
    DeserializerNode(const DeserializerNode&) = delete;
    DeserializerNode& operator=(const DeserializerNode&) = delete;
    DeserializerNode(DeserializerNode&&) = delete;
    DeserializerNode& operator=(DeserializerNode&&) = delete;

    Deserializer& _s() { return m_deserializer; }

    Type type() const;

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
        return DeserializerSStream(m_deserializer, this);
    }

    void skip();

    bool end() const;

    [[noreturn]] void throwException(const std::string& msg) const;

protected:
    // number of elements in compound object
    int length() const;
};

class DeserializerArray : public DeserializerNode
{
public:
    DeserializerArray(Deserializer& d, impl::UniqueStack* parent = nullptr);
    ~DeserializerArray();

    using DeserializerNode::length;
    DeserializerNode& index(int index);

    // intentionally hiding parent
    Type type() const { return { Type::Array }; }

    struct Query
    {
        DeserializerNode* node = nullptr;
        explicit operator bool() const { return node; }
        DeserializerNode* operator->() { return node; }
    };
    Query peeknext();
};

class DeserializerObject : private DeserializerNode
{
public:
    DeserializerObject(Deserializer& d, impl::UniqueStack* parent = nullptr);
    ~DeserializerObject();

    using DeserializerNode::_s;
    using DeserializerNode::length;
    using DeserializerNode::end;
    using DeserializerNode::throwException;

    DeserializerNode& key(std::string_view k);

    DeserializerObject obj(std::string_view k)
    {
        return key(k).obj();
    }
    DeserializerArray ar(std::string_view k)
    {
        return key(k).ar();
    }

    DeserializerNode* optkey(std::string_view k);

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
    void optval(std::string_view k, T& v)
    {
        if (auto open = optkey(k))
        {
            open->val(v);
        }
    }

    template <typename T>
    void optval(std::string_view k, std::optional<T>& v, const T& d)
    {
        if (auto open = optkey(k))
        {
            open->val(v.emplace());
        }
        else
        {
            v.reset(d);
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

    std::optional<DeserializerSStream> optsstream(std::string_view k)
    {
        if (auto open = optkey(k))
        {
            return open->sstream();
        }
        return std::nullopt;
    }

    struct KeyQuery
    {
        std::string_view name;
        DeserializerNode* node = nullptr;
        explicit operator bool() const { return node; }
        DeserializerNode* operator->() { return node; }
    };
    KeyQuery peeknext();

    template <typename Key, typename T>
    void nextkeyval(Key& k, T& v);

    // intentionally hiding parent
    Type type() const { return { Type::Object }; }
};

inline DeserializerSStream::DeserializerSStream(Deserializer& d, impl::UniqueStack* parent)
    : impl::UniqueStack(parent)
    , m_deserializer(d)
    , m_stream(&loadStringStream_msg::call(d))
{}

inline DeserializerSStream::~DeserializerSStream()
{
    if (m_stream) unloadStringStream_msg::call(m_deserializer);
}

inline void DeserializerSStream::throwException(const std::string& msg) const {
    throwDeserializerException_msg::call(m_deserializer, msg);
}

inline DeserializerObject DeserializerNode::obj()
{
    return DeserializerObject(m_deserializer, this);
}

inline DeserializerArray DeserializerNode::ar()
{
    return DeserializerArray(m_deserializer, this);
}

inline void DeserializerNode::throwException(const std::string& msg) const {
    throwDeserializerException_msg::call(m_deserializer, msg);
}

namespace impl
{
template <typename, typename = void>
struct HasPolyDeserialize : std::false_type {};
template <typename T>
struct HasPolyDeserialize<T, decltype(husePolyDeserialize(std::declval<Deserializer&>(), std::declval<T&>()))> : std::true_type {};

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
    if constexpr (impl::HasDeserializeMethod<T>::value)
    {
        v.huseDeserialize(*this);
    }
    else if constexpr (impl::HasDeserializeFunc<T>::value)
    {
        huseDeserialize(*this, v);
    }
    else if constexpr (impl::HasPolyDeserialize<T>::value)
    {
        husePolyDeserialize(m_deserializer, v);
    }
    else
    {
        cannot_deserialize(v);
    }
}

inline Type DeserializerNode::type() const
{
    return pendingType_msg::call(m_deserializer);
}

inline int DeserializerNode::length() const
{
    return curLength_msg::call(m_deserializer);
}

inline void DeserializerNode::skip()
{
    skip_msg::call(m_deserializer);
}

inline bool DeserializerNode::end() const
{
    return !hasPending_msg::call(m_deserializer);
}

inline DeserializerArray::DeserializerArray(Deserializer& d, impl::UniqueStack* parent)
    : DeserializerNode(d, parent)
{
    loadArray_msg::call(m_deserializer);
}

inline DeserializerArray::~DeserializerArray()
{
    unloadArray_msg::call(m_deserializer);
}

inline DeserializerNode& DeserializerArray::index(int index)
{
    loadIndex_msg::call(m_deserializer, index);
    return *this;
}

inline DeserializerArray::Query DeserializerArray::peeknext()
{
    if (!hasPending_msg::call(m_deserializer)) return {};
    return {this};
}

inline DeserializerObject::DeserializerObject(Deserializer& d, impl::UniqueStack* parent)
    : DeserializerNode(d, parent)
{
    loadObject_msg::call(m_deserializer);
}
inline DeserializerObject::~DeserializerObject()
{
    unloadObject_msg::call(m_deserializer);
}

inline DeserializerNode& DeserializerObject::key(std::string_view k)
{
    loadKey_msg::call(m_deserializer, k);
    return *this;
}

inline DeserializerNode* DeserializerObject::optkey(std::string_view k)
{
    if (tryLoadKey_msg::call(m_deserializer, k)) return this;
    return nullptr;
}

inline DeserializerObject::KeyQuery DeserializerObject::peeknext()
{
    auto name = optPendingKey_msg::call(m_deserializer);
    if (!name) return {};
    return {*name, this};
}

template <typename Key, typename T>
void DeserializerObject::nextkeyval(Key& k, T& v)
{
    k = Key(pendingKey_msg::call(m_deserializer));
    this->DeserializerNode::val(v);
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

inline DeserializerNode Deserializer::node() {
    return DeserializerNode(*this, nullptr);
}

inline DeserializerNode Deserializer::root() {
    return node();
}

} // namespace huse
