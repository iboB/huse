// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "JsonDeserializer.hpp"

#if defined(__GNUC__) && !defined(__clang__)
#define DISABLE_SAJSON_WARNINGS \
     _Pragma("GCC diagnostic push") \
     _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    /*preserve this line*/
#define REENABLE_SAJSON_WARNINGS _Pragma("GCC diagnostic pop")
#else
#define DISABLE_SAJSON_WARNINGS
#define REENABLE_SAJSON_WARNINGS
#endif

#include "_sajson/sajson.hpp"

#include <vector>
#include <string>
#include <cmath>
#include <exception>

namespace huse
{

JsonDeserializer JsonDeserializer::fromConstString(std::string_view str)
{
    return JsonDeserializer(
        sajson::parse(
            sajson::single_allocation(),
            sajson::string(str.data(), str.length())));
}

JsonDeserializer JsonDeserializer::fromMutableString(char* str, size_t size /*= size_t(-1)*/)
{
    if (size == size_t(-1)) size = strlen(str);
    return JsonDeserializer(
    sajson::parse(
        sajson::single_allocation(),
        sajson::mutable_string_view(size, str)));
}

struct JsonDeserializer::Impl
{
    sajson::document document;

    struct Value
    {
        sajson::value sjvalue;
        std::string_view key;
        int index = 0;
    };

    struct StackElement
    {
        Value value;
        std::optional<Value> pending;
    };
    std::vector<StackElement> stack;

    template <typename Target, typename Source>
    Target signedCheck(Source s)
    {
        if constexpr (std::is_unsigned_v<Target>)
        {
            if (s < 0) throwException("not unsigned");
        }
        return Target(s);
    }

    template <typename T>
    void readInt(T& val)
    {
        auto jval = r();
        if (jval.get_type() != sajson::TYPE_INTEGER) throwException("not integer");
        val = signedCheck<T>(jval.get_integer_value());
    }

    template <typename T>
    void readLargeInt(T& val)
    {
        auto jval = r();
        if (jval.get_type() == sajson::TYPE_INTEGER)
        {
            val = signedCheck<T>(jval.get_integer_value());
        }
        else if (jval.get_type() == sajson::TYPE_DOUBLE)
        {
            auto d = jval.get_double_value();
            double tmp;
            if (std::modf(d, &tmp) != 0) throwException("not integer");
            val = signedCheck<T>(std::floor(d));
        }
        else
        {
            throwException("not integer");
        }
    }

    template <typename T>
    void readFloat(T& val)
    {
        auto jval = r();
        if (jval.get_type() == sajson::TYPE_INTEGER) val = T(jval.get_integer_value());
        else if (jval.get_type() == sajson::TYPE_DOUBLE) val = T(jval.get_double_value());
        else throwException("not number");
    }

    template <typename S>
    void readString(S& val)
    {
        auto jval = r();
        if (jval.get_type() != sajson::TYPE_STRING) throwException("not string");
        val = {jval.as_cstring(), jval.get_string_length()};
    }

    Value advance()
    {
        if (stack.empty()) return Value{ document.get_root(), "root", 0};

        auto& top = stack.back();
        auto tt = top.value.sjvalue.get_type();
        assert(tt == sajson::TYPE_ARRAY || tt == sajson::TYPE_OBJECT);

        if (!top.pending) throwException("out of range");

        const auto current = *top.pending;
        auto nextIndex = top.pending->index + 1;

        if (int(top.value.sjvalue.get_length()) <= nextIndex)
        {
            top.pending.reset();
            return current;
        }

        auto& pending = *top.pending;
        if (tt == sajson::TYPE_ARRAY)
        {
            pending.sjvalue = top.value.sjvalue.get_array_element(nextIndex);
            pending.key = {};
        }
        else
        {
            pending.sjvalue = top.value.sjvalue.get_object_value(nextIndex);
            auto k = top.value.sjvalue.get_object_key(nextIndex);
            pending.key = {k.data(), k.length()};
        }

        pending.index = nextIndex;
        return current;
    }

    sajson::value r()
    {
        return advance().sjvalue;
    }

    bool tryLoadKey(std::string_view key)
    {
        assert(!stack.empty());

        auto& top = stack.back();

        assert(top.value.sjvalue.get_type() == sajson::TYPE_OBJECT);

        // optimistic check whether the pending key is what we actually want
        if (top.pending && top.pending->key == key) return true;

        auto k = top.value.sjvalue.find_object_key(sajson::string(key.data(), key.length()));
        if (k >= top.value.sjvalue.get_length()) return false;

        // adjust pending so the next call of advance loads it
        auto& pending = top.pending.emplace();
        pending.sjvalue = top.value.sjvalue.get_object_value(k);
        pending.key = key;
        pending.index = int(k);
        return true;
    }

    void loadKey(std::string_view key)
    {
        if (!tryLoadKey(key)) throwException(std::string(key));
    }

    std::optional<std::string_view> loadNextKey()
    {
        assert(!stack.empty());
        auto& top = stack.back();
        assert(top.value.sjvalue.get_type() == sajson::TYPE_OBJECT);
        if (top.pending) return top.pending->key;
        return std::nullopt;
    }

    void loadIndex(int index)
    {
        assert(!stack.empty());

        auto& top = stack.back();

        assert(top.value.sjvalue.get_type() == sajson::TYPE_ARRAY);

        // optimistic check whether the pending index is the same
        if (top.pending && top.pending->index == index) return;

        if (index >= int(top.value.sjvalue.get_length())) throwException(std::to_string(index));

        // adjust pending so the next call of advance loads it
        auto& pending = top.pending.emplace();
        pending.sjvalue = top.value.sjvalue.get_array_element(size_t(index));
        pending.index = index;
    }

    void loadCompound(sajson::type target)
    {
        const auto current = advance();
        if (current.sjvalue.get_type() != target)
        {
            if (target == sajson::TYPE_ARRAY) throwException("not array");
            else throwException("not object");
        }

        auto& top = stack.emplace_back();
        top.value = current;

        // adjust pending so the next call of advance loads it
        if (top.value.sjvalue.get_length() == 0) return; // empty compound, nothing to do

        auto& pending = top.pending.emplace();
        if (target == sajson::TYPE_ARRAY)
        {
            pending.sjvalue = top.value.sjvalue.get_array_element(0);
        }
        else
        {
            pending.sjvalue = top.value.sjvalue.get_object_value(0);
            auto k = top.value.sjvalue.get_object_key(0);
            pending.key = {k.data(), k.length()};
        }

        pending.index = 0;
    }

    void unloadCompound()
    {
        stack.pop_back();
    }

    int curLength() const
    {
        if (stack.empty()) return 1;
        return int(stack.back().value.sjvalue.get_length());
    }

    [[noreturn]] void throwException(std::string msg) const
    {
        throw msg;
    }
};

JsonDeserializer::JsonDeserializer(sajson::document&& doc)
    : m_i(new Impl{std::move(doc), {}})
{
    if (!m_i->document.is_valid())
    {
        throwException(m_i->document.get_error_message_as_cstring());
    }
}

JsonDeserializer::~JsonDeserializer()
{
    if (std::uncaught_exceptions()) return;
    assert(m_i->stack.size() == 0);
}

void JsonDeserializer::read(bool& val)
{
    auto t = m_i->r().get_type();
    if (t == sajson::TYPE_TRUE) val = true;
    else if (t == sajson::TYPE_FALSE) val = false;
    else throwException("not bool");
}

void JsonDeserializer::read(short& val) { m_i->readInt(val); }
void JsonDeserializer::read(unsigned short& val) { m_i->readInt(val); }
void JsonDeserializer::read(int& val) { m_i->readInt(val); }
void JsonDeserializer::read(unsigned int& val) { m_i->readLargeInt(val); }
void JsonDeserializer::read(long& val)
{
    if constexpr (sizeof(long) == 4)
    {
        // gcc and clang have long equal intptr_t, msvc has long at 4 bytes
        m_i->readInt(val);
    }
    else
    {
        m_i->readLargeInt(val);
    }
}
void JsonDeserializer::read(unsigned long& val) { m_i->readLargeInt(val); }
void JsonDeserializer::read(long long& val) { m_i->readLargeInt(val); }
void JsonDeserializer::read(unsigned long long& val) { m_i->readLargeInt(val); }
void JsonDeserializer::read(float& val) { m_i->readFloat(val); }
void JsonDeserializer::read(double& val) { m_i->readFloat(val); }
void JsonDeserializer::read(std::string_view& val) { m_i->readString(val); }
void JsonDeserializer::read(std::string& val) { m_i->readString(val); }

void JsonDeserializer::loadObject() { m_i->loadCompound(sajson::TYPE_OBJECT); }
void JsonDeserializer::unloadObject() { m_i->unloadCompound(); }
void JsonDeserializer::loadArray() { m_i->loadCompound(sajson::TYPE_ARRAY); }
void JsonDeserializer::unloadArray() { m_i->unloadCompound(); }

int JsonDeserializer::curLength() const { return m_i->curLength(); }

void JsonDeserializer::loadKey(std::string_view key) { m_i->loadKey(key); }
bool JsonDeserializer::tryLoadKey(std::string_view key) { return m_i->tryLoadKey(key); }
void JsonDeserializer::loadIndex(int index) { m_i->loadIndex(index); }

std::optional<std::string_view> JsonDeserializer::loadNextKey() { return m_i->loadNextKey(); }

void JsonDeserializer::throwException(std::string msg) const { m_i->throwException(std::move(msg)); }

}
