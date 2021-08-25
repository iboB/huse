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

namespace huse
{

class OpenedObject;
class OpenedArray;
class Serializer;

class OpenedElement : public impl::UniqueStack
{
protected:
    OpenedElement(Serializer& s, impl::UniqueStack* parent)
        : impl::UniqueStack(parent)
        , m_serializer(s)
    {}

    Serializer& m_serializer;
public:
    OpenedElement(const OpenedElement&) = delete;
    OpenedElement operator=(const OpenedElement&) = delete;
    OpenedElement(OpenedElement&&) = delete;
    OpenedElement operator=(OpenedElement&&) = delete;

    OpenedObject obj();
    OpenedArray ar();

    template <typename T>
    void val(const T& v);
};

class OpenedArray : public OpenedElement
{
public:
    OpenedArray(Serializer& s, impl::UniqueStack* parent = nullptr);
    ~OpenedArray();
};

class OpenedObject : public OpenedElement
{
public:
    OpenedObject(Serializer& s, impl::UniqueStack* parent = nullptr);
    ~OpenedObject();

    OpenedObject obj(std::string_view k)
    {
        key(k);
        return OpenedElement::obj();
    }
    OpenedArray ar(std::string_view k)
    {
        key(k);
        return OpenedElement::ar();
    }

    template <typename T>
    void val(std::string_view k, const T& v) {
        key(k);
        OpenedElement::val(v);
    }

    template <typename T>
    void val(std::string_view k, const std::optional<T>& v)
    {
        if (v)
        {
            key(k);
            OpenedElement::val(*v);
        }
    }

    template <typename T>
    void optval(std::string_view k, const T& v) { val(k, v); } // compatibility with deserializer

    template <typename T>
    void optval(std::string_view k, const std::optional<T>& v, const T& d)
    {
        if (!v)
        {
            val(k, d);
            return;
        }
        val(k, *v);
    }

private:
    void key(std::string_view k);
};

class HUSE_API Serializer : public OpenedElement {
    friend class OpenedArray;
    friend class OpenedObject;
public:
    Serializer() : OpenedElement(*this, nullptr) {}
    virtual ~Serializer();

    struct HUSE_API Exception {};

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

    // helpers
    void write(const char* s) { write(std::string_view(s)); }

protected:
    // implementation interface
    virtual void pushKey(std::string_view k) = 0;

    virtual void openObject() = 0;
    virtual void closeObject() = 0;

    virtual void openArray() = 0;
    virtual void closeArray() = 0;
};

inline OpenedObject OpenedElement::obj()
{
    return OpenedObject(m_serializer, this);
}

inline OpenedArray OpenedElement::ar()
{
    return OpenedArray(m_serializer, this);
}

namespace impl
{
template <typename, typename = void>
struct HasSerializerWrite : std::false_type {};

template <typename T>
struct HasSerializerWrite<T, decltype(std::declval<Serializer>().write(std::declval<T>()))> : std::true_type {};

template <typename, typename = void>
struct HasSerializeMethod : std::false_type {};

template <typename T>
struct HasSerializeMethod<T, decltype(std::declval<T>().huseSerialize(std::declval<Serializer&>()))> : std::true_type {};

template <typename, typename = void>
struct HasSerializeFunc : std::false_type {};

template <typename T>
struct HasSerializeFunc<T, decltype(huseSerialize(std::declval<Serializer&>(), std::declval<T>()))> : std::true_type {};
} // namespace impl

template <typename T>
void OpenedElement::val(const T& v)
{
    if constexpr (impl::HasSerializeMethod<T>::value)
    {
        v.huseSerialize(m_serializer);
    }
    else if constexpr (impl::HasSerializeFunc<T>::value)
    {
        huseSerialize(m_serializer, v);
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

inline OpenedArray::OpenedArray(Serializer& s, impl::UniqueStack* parent)
    : OpenedElement(s, parent)
{
    m_serializer.openArray();
}

inline OpenedArray::~OpenedArray()
{
    m_serializer.closeArray();
}

inline OpenedObject::OpenedObject(Serializer& s, impl::UniqueStack* parent)
    : OpenedElement(s, parent)
{
    m_serializer.openObject();
}

inline OpenedObject::~OpenedObject()
{
    m_serializer.closeObject();
}

inline void OpenedObject::key(std::string_view k)
{
    m_serializer.pushKey(k);
}

} // namespace huse
