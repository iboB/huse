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

#include "Context.hpp"

#include <string_view>
#include <optional>

#include <iosfwd>

namespace huse
{

class DeserializerArray;
class DeserializerObject;
class BasicDeserializer;

struct Type
{
public:
    enum Value : int
    {
        True = 0b01,
        False = 0b10,
        Boolean = 0b11,
        Integer = 0b0100,
        Float = 0b1000,
        Number = 0b1100,
        String = 0b10000,
        Object = 0b100000,
        Array = 0b1000000,
        Null = 0b10000000,
    };

    Type(Value t) : m_t(t) {}

    bool is(Value mask) const { return m_t & mask; }
private:
    Value m_t;
};

class DeserializerSStream : public impl::UniqueStack
{
public:
    DeserializerSStream(BasicDeserializer& d, impl::UniqueStack* parent);
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

private:
    BasicDeserializer& m_deserializer;
    std::istream* m_stream;
};

class DeserializerNode : public impl::UniqueStack
{
protected:
    DeserializerNode(BasicDeserializer& d, impl::UniqueStack* parent)
        : impl::UniqueStack(parent)
        , m_deserializer(d)
    {}

    BasicDeserializer& m_deserializer;
public:
    DeserializerNode(const DeserializerNode&) = delete;
    DeserializerNode& operator=(const DeserializerNode&) = delete;
    DeserializerNode(DeserializerNode&&) = delete;
    DeserializerNode& operator=(DeserializerNode&&) = delete;

    Context context() const;

    Type type() const;

    DeserializerObject obj();
    DeserializerArray ar();

    template <typename T>
    void val(T& v);

    template <typename T, typename F>
    void cval(T& v, F f)
    {
        f(*this, v);
    }

    DeserializerSStream sstream()
    {
        return DeserializerSStream(m_deserializer, this);
    }

    void skip();

protected:
    // number of elements in compound object
    int length() const;
};

class DeserializerArray : public DeserializerNode
{
public:
    DeserializerArray(BasicDeserializer& d, impl::UniqueStack* parent = nullptr);
    ~DeserializerArray();

    using DeserializerNode::length;
    DeserializerNode& index(int index);

    // intentionally hiding parent
    Type type() const { return { Type::Array }; }
};

class DeserializerObject : private DeserializerNode
{
public:
    DeserializerObject(BasicDeserializer& d, impl::UniqueStack* parent = nullptr);
    ~DeserializerObject();

    using DeserializerNode::length;

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
    void cval(std::string_view k, T& v, F f)
    {
        key(k).cval(v, std::move(f));
    }

    template <typename T, typename F>
    void cval(std::string_view k, std::optional<T>& v, F f)
    {
        if (auto open = optkey(k))
        {
            open->cval(v.emplace(), std::move(f));
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

class HUSE_API BasicDeserializer : public DeserializerNode
{
    friend class DeserializerSStream;
    friend class DeserializerNode;
    friend class DeserializerArray;
    friend class DeserializerObject;
public:
    BasicDeserializer(Context ctx) : DeserializerNode(*this, nullptr), m_context(ctx) {}
    virtual ~BasicDeserializer();

    [[noreturn]] virtual void throwException(const std::string& msg) const;

protected:
    // read interface
    virtual void read(bool& val) = 0;
    virtual void read(short& val) = 0;
    virtual void read(unsigned short& val) = 0;
    virtual void read(int& val) = 0;
    virtual void read(unsigned int& val) = 0;
    virtual void read(long& val) = 0;
    virtual void read(unsigned long& val) = 0;
    virtual void read(long long& val) = 0;
    virtual void read(unsigned long long& val) = 0;
    virtual void read(float& val) = 0;
    virtual void read(double& val) = 0;
    virtual void read(std::string_view& val) = 0;
    virtual void read(std::string& val) = 0;

    // skip a value
    virtual void skip() = 0;

    // stateful reads
    virtual std::istream& loadStringStream() = 0;
    virtual void unloadStringStream() = 0;

    // implementation interface
    virtual void loadObject() = 0;
    virtual void unloadObject() = 0;

    virtual void loadArray() = 0;
    virtual void unloadArray() = 0;

    // number of sub-nodes in current node
    virtual int curLength() const = 0;

    // throw if no key
    virtual void loadKey(std::string_view key) = 0;

    // load and resturn true if key exists, otherwise return false
    // equivalent to (but more optimized than)
    // if (hasKey(k)) { loadKey(k); return true; } else return false;
    virtual bool tryLoadKey(std::string_view key) = 0;

    // throw if no index
    virtual void loadIndex(int index) = 0;

    virtual Type pendingType() const = 0;

    // throw if no pending key
    virtual std::string_view pendingKey() = 0;

    // return pending key or nullopt if there is none
    virtual std::optional<std::string_view> optPendingKey() = 0;

private:
    Context m_context;
};

inline DeserializerSStream::DeserializerSStream(BasicDeserializer& d, impl::UniqueStack* parent)
    : impl::UniqueStack(parent)
    , m_deserializer(d)
    , m_stream(&d.loadStringStream())
{}

inline DeserializerSStream::~DeserializerSStream()
{
    if (m_stream) m_deserializer.unloadStringStream();
}

inline DeserializerObject DeserializerNode::obj()
{
    return DeserializerObject(m_deserializer, this);
}

inline DeserializerArray DeserializerNode::ar()
{
    return DeserializerArray(m_deserializer, this);
}

namespace impl
{
struct DeserializerReadHelper : public BasicDeserializer {
    using BasicDeserializer::read;
};

template <typename, typename = void>
struct HasDeserializerRead : std::false_type {};

template <typename T>
struct HasDeserializerRead<T, decltype(std::declval<DeserializerReadHelper>().read(std::declval<T&>()))> : std::true_type {};

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
    else if constexpr (impl::HasDeserializerRead<T>::value)
    {
        m_deserializer.read(v);
    }
    else
    {
        cannot_deserialize(v);
    }
}

inline Context DeserializerNode::context() const
{
    return m_deserializer.m_context;
}

inline Type DeserializerNode::type() const
{
    return m_deserializer.pendingType();
}

inline int DeserializerNode::length() const
{
    return m_deserializer.curLength();
}

inline void DeserializerNode::skip() {
    m_deserializer.skip();
}

inline DeserializerArray::DeserializerArray(BasicDeserializer& d, impl::UniqueStack* parent)
    : DeserializerNode(d, parent)
{
    m_deserializer.loadArray();
}

inline DeserializerArray::~DeserializerArray()
{
    m_deserializer.unloadArray();
}

inline DeserializerNode& DeserializerArray::index(int index)
{
    m_deserializer.loadIndex(index);
    return *this;
}

inline DeserializerObject::DeserializerObject(BasicDeserializer& d, impl::UniqueStack* parent)
    : DeserializerNode(d, parent)
{
    m_deserializer.loadObject();
}
inline DeserializerObject::~DeserializerObject()
{
    m_deserializer.unloadObject();
}

inline DeserializerNode& DeserializerObject::key(std::string_view k)
{
    m_deserializer.loadKey(k);
    return *this;
}

inline DeserializerNode* DeserializerObject::optkey(std::string_view k)
{
    if (m_deserializer.tryLoadKey(k)) return this;
    return nullptr;
}

inline DeserializerObject::KeyQuery DeserializerObject::peeknext()
{
    auto name = m_deserializer.optPendingKey();
    if (!name) return {};
    return {*name, this};
}

template <typename Key, typename T>
void DeserializerObject::nextkeyval(Key& k, T& v)
{
    k = Key(m_deserializer.pendingKey());
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

} // namespace huse
