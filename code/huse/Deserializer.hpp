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

class DeserializerArray;
class DeserializerObject;
class Deserializer;

class DeserializerNode : public impl::UniqueStack
{
protected:
    DeserializerNode(Deserializer& d, impl::UniqueStack* parent)
        : impl::UniqueStack(parent)
        , m_deserializer(d)
    {}

    Deserializer& m_deserializer;
public:
    DeserializerNode(const DeserializerNode&) = delete;
    DeserializerNode operator=(const DeserializerNode&) = delete;
    DeserializerNode(DeserializerNode&&) = delete;
    DeserializerNode operator=(DeserializerNode&&) = delete;

    DeserializerObject obj();
    DeserializerArray ar();

    template <typename T>
    void val(T& v);

protected:
    // number of elements in compound object
    size_t length() const;
};

class DeserializerArray : public DeserializerNode
{
public:
    DeserializerArray(Deserializer& d, impl::UniqueStack* parent = nullptr);
    ~DeserializerArray();

    using DeserializerNode::length;
    DeserializerNode& index(size_t index);

};

class DeserializerObject : private DeserializerNode
{
public:
    DeserializerObject(Deserializer& d, impl::UniqueStack* parent = nullptr);
    ~DeserializerObject();

    using DeserializerNode::length;

    DeserializerNode& key(std::string_view k);

    DeserializerObject obj(std::string_view k);
    DeserializerArray ar(std::string_view k);
};

class HUSE_API Deserializer
{
public:
    virtual ~Deserializer();

    struct HUSE_API Exception {};
    [[noreturn]] virtual void throwException(std::string msg) const = 0;

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

    // implementation interface
    virtual void loadObject() = 0;
    virtual void unloadObject() = 0;

    virtual void loadArray() = 0;
    virtual void unloadArray() = 0;

    // number of subNodes in current node
    virtual size_t curLength() = 0;

    // throw if no key
    virtual void loadKey(std::string_view key) = 0;

    // load and resturn true if key exists, otherwise return false
    // equivalent to (but more optimized than)
    // if (hasKey(k)) { loadKey(k); return true; } else return false;
    virtual bool tryLoadKey(std::string_view key) = 0;

    // throw if no index
    virtual void loadIndex(int index) = 0;

    // noad the next key and return it or nullopt if there is no next key
    virtual std::optional<std::string_view> loadNextKey() = 0;
};

}
