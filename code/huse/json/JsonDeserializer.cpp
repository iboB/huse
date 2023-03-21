// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//

#include "../DeserializerObj.hpp"
#include "../DeserializerInterface.hpp"
#include "../Domain.hpp"
#include "../Exception.hpp"
#include "../PolyTraits.hpp"
#include "../impl/Assert.hpp"

#include "_sajson/sajson.hpp"

#include <itlib/mem_streambuf.hpp>

#include <dynamix/define_mixin.hpp>
#include <dynamix/mutate.hpp>

#include <vector>
#include <string>
#include <cmath>
#include <sstream>

namespace huse::json
{

namespace
{
constexpr std::string_view Not_Integer = "not an integer";
constexpr std::string_view Out_of_Range = "out of range";

struct MemIStream
{
    MemIStream(std::string_view str)
        : streambuf(str.data(), str.size())
        , stream(&streambuf)
    {}

    itlib::mem_istreambuf<char> streambuf;
    std::istream stream;
};
}

//Deserializer::Deserializer(std::string_view str, Context* ctx)
//    : Deserializer(
//        sajson::parse(
//            sajson::single_allocation(),
//            sajson::string(str.data(), str.length())),
//        ctx
//    )
//{}
//
//Deserializer Deserializer::fromConstString(std::string_view str, Context* ctx)
//{
//    return Deserializer(str, ctx);
//}
//
//Deserializer::Deserializer(char* str, size_t size /*= size_t(-1)*/, Context* ctx)
//    : Deserializer(
//        sajson::parse(
//            sajson::single_allocation(),
//            sajson::mutable_string_view(size == size_t(-1) ? strlen(str) : size, str)),
//        ctx
//    )
//{}
//
//Deserializer Deserializer::fromMutableString(char* str, size_t size /*= size_t(-1)*/, Context* ctx)
//{
//    return Deserializer(str, size, ctx);
//}

struct JsonDeserializer
{
    sajson::document document;

    struct Value
    {
        sajson::value sjvalue;
        std::string_view key;
        int index;
    };

    struct StackElement
    {
        Value value;
        std::optional<Value> pending;
    };
    std::vector<StackElement> stack;

    Value current; // only valid after advance

    std::optional<MemIStream> m_stringStream;

    JsonDeserializer(sajson::document&& doc)
        : document(std::move(doc))
    {}

    ~JsonDeserializer() {
        HUSE_ASSERT_INTERNAL(stack.size() == 0);
    }

    template <typename Target, typename Source>
    Target signedCheck(Source s)
    {
        if constexpr (std::is_unsigned_v<Target>)
        {
            if (s < 0) throwException("negative integer");
        }
        return Target(s);
    }

    template <typename T>
    void readInt(T& val)
    {
        auto jval = r();
        if (jval.get_type() != sajson::TYPE_INTEGER) throwException(Not_Integer);
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
            if (std::modf(d, &tmp) != 0) throwException(Not_Integer);
            val = signedCheck<T>(std::floor(d));
        }
        else
        {
            throwException(Not_Integer);
        }
    }

    template <typename T>
    void readFloat(T& val)
    {
        auto jval = r();
        if (jval.get_type() == sajson::TYPE_INTEGER) val = T(jval.get_integer_value());
        else if (jval.get_type() == sajson::TYPE_DOUBLE) val = T(jval.get_double_value());
        else throwException("not a number");
    }

    template <typename S>
    void readString(S& val)
    {
        auto jval = r();
        if (jval.get_type() != sajson::TYPE_STRING) throwException("not a string");
        val = {jval.as_cstring(), jval.get_string_length()};
    }

    void advance()
    {
        if (stack.empty())
        {
            current = {document.get_root(), "root", 0};
            return;
        }

        auto& top = stack.back();
        auto tt = top.value.sjvalue.get_type();
        HUSE_ASSERT_INTERNAL(tt == sajson::TYPE_ARRAY || tt == sajson::TYPE_OBJECT);

        if (!top.pending)
        {
            // "hacky" adjust current so that the exception stack printer does something nice
            current.key = {};
            current.index = int(top.value.sjvalue.get_length());
            throwException(Out_of_Range);
        }

        current = *top.pending;
        auto nextIndex = top.pending->index + 1;

        if (int(top.value.sjvalue.get_length()) <= nextIndex)
        {
            top.pending.reset();
            return;
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
    }

    const sajson::value& r()
    {
        advance();
        return current.sjvalue;
    }

    bool tryLoadKey(std::string_view key)
    {
        HUSE_ASSERT_INTERNAL(!stack.empty());

        auto& top = stack.back();

        HUSE_ASSERT_INTERNAL(top.value.sjvalue.get_type() == sajson::TYPE_OBJECT);

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
        if (!tryLoadKey(key))
        {
            // "hacky" adjust current so that the exception stack printer does something nice
            current.key = key;
            throwException(Out_of_Range);
        }
    }

    bool hasPending() const
    {
        if (stack.empty()) return true; // root is pending
        return !!stack.back().pending;
    }

    std::string_view pendingKey()
    {
        auto t = optPendingKey();
        if (t) return *t;
        // "hacky" adjust current so that the exception stack printer does something nice
        current.key = {};
        current.index = int(stack.back().value.sjvalue.get_length());
        throwException(Out_of_Range);
    }

    std::optional<std::string_view> optPendingKey() const
    {
        HUSE_ASSERT_INTERNAL(!stack.empty());
        auto& top = stack.back();
        HUSE_ASSERT_INTERNAL(top.value.sjvalue.get_type() == sajson::TYPE_OBJECT);
        if (top.pending) return top.pending->key;
        return std::nullopt;
    }

    void loadIndex(int index)
    {
        HUSE_ASSERT_INTERNAL(!stack.empty());

        auto& top = stack.back();

        HUSE_ASSERT_INTERNAL(top.value.sjvalue.get_type() == sajson::TYPE_ARRAY);

        // optimistic check whether the pending index is the same
        if (top.pending && top.pending->index == index) return;

        if (index >= int(top.value.sjvalue.get_length())) {
            // "hacky" adjust current so that the exception stack printer does something nice
            current.key = {};
            current.index = index;
            throwException(Out_of_Range);
        }

        // adjust pending so the next call of advance loads it
        auto& pending = top.pending.emplace();
        pending.sjvalue = top.value.sjvalue.get_array_element(size_t(index));
        pending.index = index;
    }

    void loadCompound(sajson::type target)
    {
        advance();
        if (current.sjvalue.get_type() != target)
        {
            if (target == sajson::TYPE_ARRAY) throwException("not an array");
            else throwException("not an object");
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

    static Type fromSajsonType(sajson::type t)
    {
        switch (t)
        {
        case sajson::TYPE_INTEGER: return {Type::Integer};
        case sajson::TYPE_DOUBLE:  return {Type::Float};
        case sajson::TYPE_NULL:    return {Type::Null};
        case sajson::TYPE_FALSE:   return {Type::False};
        case sajson::TYPE_TRUE:    return {Type::True};
        case sajson::TYPE_STRING:  return {Type::String};
        case sajson::TYPE_ARRAY:   return {Type::Array};
        case sajson::TYPE_OBJECT:  return {Type::Object};
        default:
            HUSE_ASSERT_INTERNAL(false);
            return {Type::Null};
        }
    }

    Type pendingType() const
    {
        if (stack.empty()) return fromSajsonType(document.get_root().get_type());

        auto& top = stack.back();
        HUSE_ASSERT_INTERNAL(top.pending);
        //if (!top.pending) throwException(Out_of_Range);
        return fromSajsonType(top.pending->sjvalue.get_type());
    }

    [[noreturn]] void throwException(const std::string_view msg) const
    {
        std::ostringstream sout;

        if (!stack.empty())
        {
            auto i = stack.begin();
            sout << i->value.key; // don't wrap root in quotes

            auto printStackItem = [&sout](const Value& val) {
                sout << '.';
                if (val.key.empty()) sout << '[' << val.index << ']';
                else sout << '"' << val.key << '"';
            };

            for (++i; i!=stack.end(); ++i)
            {
                printStackItem(i->value);
            }
            printStackItem(current);
        }
        else
        {
            // certainly this is root
            sout << current.key;
        }

        sout << " : " << msg;
        throw DeserializerException(sout.str());
    }

    std::istream& loadStringStream()
    {
        std::string_view cur;
        readString(cur);
        HUSE_ASSERT_INTERNAL(!m_stringStream);
        m_stringStream.emplace(cur);
        return m_stringStream->stream;
    }

    void unloadStringStream()
    {
        assert(!!m_stringStream);
        m_stringStream.reset();
    }

    void husePolyDeserialize(bool& val) {
        auto t = r().get_type();
        if (t == sajson::TYPE_TRUE) val = true;
        else if (t == sajson::TYPE_FALSE) val = false;
        else throwException("not a boolean");
    }
    void husePolyDeserialize(short& val) {
        readInt(val);
    }
    void husePolyDeserialize(unsigned short& val) {
        readInt(val);
    }
    void husePolyDeserialize(int& val) {
        readInt(val);
    }
    void husePolyDeserialize(unsigned int& val) {
        readLargeInt(val);
    }
    void husePolyDeserialize(long& val) {
        if constexpr (sizeof(long) == 4) {
            // gcc and clang have long equal intptr_t, msvc has long at 4 bytes
            readInt(val);
        }
        else {
            readLargeInt(val);
        }
    }
    void husePolyDeserialize(unsigned long& val) {
        readLargeInt(val);
    }
    void husePolyDeserialize(long long& val) {
        readLargeInt(val);
    }
    void husePolyDeserialize(unsigned long long& val) {
        readLargeInt(val);
    }
    void husePolyDeserialize(float& val) {
        readFloat(val);
    }
    void husePolyDeserialize(double& val) {
        readFloat(val);
    }
    void husePolyDeserialize(std::string_view& val) {
        readString(val);
    }
    void husePolyDeserialize(std::string& val) {
        readString(val);
    }
};

DYNAMIX_DEFINE_MIXIN(DeserializerDomain, JsonDeserializer)
.implements<husePolyDeserialize_bool>()
.implements<husePolyDeserialize_short>()
.implements<husePolyDeserialize_ushort>()
.implements<husePolyDeserialize_int>()
.implements<husePolyDeserialize_uint>()
.implements<husePolyDeserialize_long>()
.implements<husePolyDeserialize_ulong>()
.implements<husePolyDeserialize_llong>()
.implements<husePolyDeserialize_ullong>()
.implements<husePolyDeserialize_float>()
.implements<husePolyDeserialize_double>()
.implements<husePolyDeserialize_sv>()
.implements<husePolyDeserialize_string>()
.implements_by<skip_msg>([](JsonDeserializer* d) { d->advance(); })
.implements_by<loadStringStream_msg>([](JsonDeserializer* d) -> std::istream& { return d->loadStringStream(); })
.implements_by<unloadStringStream_msg>([](JsonDeserializer* d) { d->unloadStringStream(); })
.implements_by<loadObject_msg>([](JsonDeserializer* d) { d->loadCompound(sajson::TYPE_OBJECT); })
.implements_by<unloadObject_msg>([](JsonDeserializer* d) { d->unloadCompound(); })
.implements_by<loadArray_msg>([](JsonDeserializer* d) { d->loadCompound(sajson::TYPE_ARRAY); })
.implements_by<unloadArray_msg>([](JsonDeserializer* d) { d->unloadCompound(); })
.implements_by<curLength_msg>([](const JsonDeserializer* d) { return d->curLength(); })
.implements_by<loadKey_msg>([](JsonDeserializer* d, std::string_view key) { d->loadKey(key); })
.implements_by<tryLoadKey_msg>([](JsonDeserializer* d, std::string_view key) { return d->tryLoadKey(key); })
.implements_by<loadIndex_msg>([](JsonDeserializer* d, int index) { d->loadIndex(index); })
.implements_by<hasPending_msg>([](const JsonDeserializer* d) { return d->hasPending(); })
.implements_by<pendingType_msg>([](const JsonDeserializer* d) { return d->pendingType(); })
.implements_by<pendingKey_msg>([](const JsonDeserializer* d) { return const_cast<JsonDeserializer*>(d)->pendingKey(); })
.implements_by<optPendingKey_msg>([](const JsonDeserializer* d) { return d->optPendingKey(); })
.implements_by<throwDeserializerException_msg>([](const JsonDeserializer* d, const std::string& msg) { d->throwException(msg); })
;

/*
Deserializer::Deserializer(sajson::document&& doc, Context* ctx)
    : BasicDeserializer(ctx)
    , d(new Impl{std::move(doc), {}, {}, {}})
{
    if (!d->document.is_valid())
    {
        // don't use d->throwException because it adds the stack
        // we certainly don't have a stack here
        throw DeserializerException(d->document.get_error_message_as_cstring());
    }
}

*/

}
