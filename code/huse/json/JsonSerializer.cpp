// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "JsonSerializer.hpp"
#include "Limits.hpp"

#include "../SerializerObj.hpp"
#include "../SerializerInterface.hpp"
#include "../Domain.hpp"
#include "../Exception.hpp"
#include "../PolyTraits.hpp"
#include "../impl/Assert.hpp"
#include "../impl/Charconv.hpp"

#include <dynamix/define_mixin.hpp>
#include <dynamix/mutate.hpp>

#include <cmath>
#include <exception>
#include <new>
#include <ostream>
#include <type_traits>

namespace huse::json
{

namespace
{
std::optional<std::string_view> escapeUtf8Byte(char c)
{
    auto u = uint8_t(c);

    // http://www.json.org/
    if (u > '\\') return {}; // no escape needed for characters above backslash
    if (u == '"') return "\\\"";
    if (u == '\\') return "\\\\";
    if (u >= ' ') return {}; // no escape needed for other characters above space
    static constexpr std::string_view belowSpace[' '] = {
        "\\u0000","\\u0001","\\u0002","\\u0003","\\u0004","\\u0005","\\u0006","\\u0007",
          "\\b"  ,  "\\t"  ,  "\\n"  ,"\\u000b",  "\\f"  ,  "\\r"  ,"\\u000e","\\u000f",
        "\\u0010","\\u0011","\\u0012","\\u0013","\\u0014","\\u0015","\\u0016","\\u0017",
        "\\u0018","\\u0019","\\u001a","\\u001b","\\u001c","\\u001d","\\u001e","\\u001f"};
    return belowSpace[u];
}

void writeEscapedUTF8StringToStreambuf(std::streambuf& buf, std::string_view str)
{
    // we could use this simple code here
    // but it writes bytes one by one
    //
    // for (auto c : str)
    // {
    //     auto e = escapeUtf8Byte(c);
    //     if (!e) m_out.put(c);
    //     else m_out << *e;
    // }
    //
    // to optimize, we'll use the following which writes in chunks
    // if there is nothing to be escaped in a string,
    //  it will print the whole string at the end as a single operation

    auto begin = str.data();
    const auto end = str.data() + str.size();

    auto p = begin;
    while (p != end) {
        auto esc = escapeUtf8Byte(*p);
        if (!esc) ++p;
        else
        {
            if (p != begin) buf.sputn(begin, p - begin);
            buf.sputn(esc->data(), esc->length());
            begin = ++p;
        }
    }
    if (p != begin) buf.sputn(begin, p - begin);
}

struct JsonRedirectStreambuf : public std::streambuf
{
    JsonRedirectStreambuf(std::streambuf& redirectTarget) : m_redirectTarget(redirectTarget) {}

    int_type overflow(int_type ch) override
    {
        auto esc = escapeUtf8Byte(char(ch));
        if (esc)
        {
            m_redirectTarget.sputn(esc->data(), esc->length());
        }
        else
        {
            m_redirectTarget.sputc(char(ch));
        }

        return ch;
    }

    std::streamsize xsputn(const char_type* s, std::streamsize num) override
    {
        writeEscapedUTF8StringToStreambuf(m_redirectTarget, std::string_view(s, num));
        return num;
    }

    [[noreturn]] void throwSeekException()
    {
        throw SerializerException("Seek is not supported by JSON string streams");
    }

    [[noreturn]] pos_type seekpos(pos_type, std::ios_base::openmode) override
    {
        throwSeekException();
    }

    [[noreturn]] pos_type seekoff(off_type, std::ios_base::seekdir, std::ios_base::openmode) override
    {
        throwSeekException();
    }

    std::streambuf& m_redirectTarget;
};

struct JsonOStream
{
    JsonOStream(std::ostream& rt)
        : streambuf(*rt.rdbuf())
        , stream(&streambuf)
    {}

    JsonRedirectStreambuf streambuf;
    std::ostream stream;
};
}

struct JsonSerializer
{
    JsonSerializer(std::ostream& out, bool pretty)
        : m_out(out)
        , m_pretty(pretty)
    {}

    ~JsonSerializer() {
        if (std::uncaught_exceptions()) return; // nothing smart to do
        HUSE_ASSERT_INTERNAL(m_depth == 0);
    }
    std::ostream& m_out;

    void writeRawJson(std::string_view json)
    {
        prepareWriteVal();
        m_out.rdbuf()->sputn(json.data(), json.size());
    }

    template <typename T>
    void writeSmallInteger(T n)
    {
        prepareWriteVal();

        auto& out = *m_out.rdbuf();

        using Unsigned = std::make_unsigned_t<T>;
        Unsigned uvalue = Unsigned(n);

        if constexpr (std::is_signed_v<T>) {
            if (n < 0) {
                out.sputc('-');
                uvalue = 0 - uvalue;
            }
        }

        char buf[24]; // enough for signed 2^64 in decimal
        const auto end = buf + sizeof(buf);
        auto p = end;

        do {
            *--p = char('0' + uvalue % 10);
            uvalue /= 10;
        } while (uvalue != 0);

        out.sputn(p, end - p);
    }

    void husePolySerialize(bool val)
    {
        static constexpr std::string_view t = "true", f = "false";
        writeRawJson(val ? t : f);
    }

    void husePolySerialize(std::nullptr_t) { writeRawJson("null"); }

    void husePolySerialize(short val) { writeSmallInteger(val); }
    void husePolySerialize(unsigned short val) { writeSmallInteger(val); }
    void husePolySerialize(int val) { writeSmallInteger(val); }
    void husePolySerialize(unsigned int val) { writeSmallInteger(val); }

    template <typename T>
    void writePotentiallyBigIntegerValue(T val)
    {
        static const std::string_view IntegerTooBig = "Integer value is bigger than maximum allowed for JSON";

        if constexpr (sizeof(T) <= 4)
        {
            // gcc and clang have long equal intptr_t, msvc has long at 4 bytes
            writeSmallInteger(val);
        }
        else if constexpr (std::is_signed_v<T>)
        {
            if (val >= Min_Int64 && val <= Max_Int64)
            {
                writeSmallInteger(val);
            }
            else
            {
                throwException(std::string(IntegerTooBig));
            }
        }
        else
        {
            if (val <= Max_Uint64)
            {
                writeSmallInteger(val);
            }
            else
            {
                throwException(std::string(IntegerTooBig));
            }
        }

    }

    // some values may not fit json's numbers
    void husePolySerialize(long val) { writePotentiallyBigIntegerValue(val); }
    void husePolySerialize(unsigned long val) { writePotentiallyBigIntegerValue(val); }
    void husePolySerialize(long long val) { writePotentiallyBigIntegerValue(val); }
    void husePolySerialize(unsigned long long val) { writePotentiallyBigIntegerValue(val); }

    template <typename T>
    void writeFloatValue(T val)
    {
        if (std::isfinite(val))
        {
            char out[25]; // max length of double
            auto result = HUSE_CHARCONV_NAMESPACE::to_chars(out, out + sizeof(out), val);
            writeRawJson(std::string_view(out, result.ptr - out));
        }
        else
        {
            throwException("Floating point value is not finite. Not supported by JSON");
        }
    }

    void husePolySerialize(float val) { writeFloatValue(val); }
    void husePolySerialize(double val) { writeFloatValue(val); }

    void writeQuotedEscapedUTF8String(std::string_view str)
    {
        auto& out = *m_out.rdbuf();
        out.sputc('"');
        writeEscapedUTF8StringToStreambuf(out, str);
        out.sputc('"');
    }

    void husePolySerialize(std::string_view val)
    {
        prepareWriteVal();
        writeQuotedEscapedUTF8String(val);
    }

    void husePolySerialize(std::nullopt_t)
    {
        m_pendingKey.reset();
    }

    void pushKey(std::string_view k)
    {
        HUSE_ASSERT_INTERNAL(!m_pendingKey);
        m_pendingKey = k;
    }

    void open(char o)
    {
        prepareWriteVal();
        m_out.rdbuf()->sputc(o);
        m_hasValue = false;
        ++m_depth;
    }

    void close(char c)
    {
        HUSE_ASSERT_INTERNAL(m_depth);
        --m_depth;
        if (m_hasValue) newLine();
        m_out.rdbuf()->sputc(c);
        m_hasValue = true;
    }

    void openObject() { open('{'); }
    void closeObject() { close('}'); }
    void openArray() { open('['); }
    void closeArray() { close(']'); }

    void prepareWriteVal()
    {
        auto& out = *m_out.rdbuf();

        if (m_hasValue)
        {
            out.sputc(',');
        }

        newLine();

        if (m_pendingKey)
        {
            writeQuotedEscapedUTF8String(*m_pendingKey);
            out.sputc(':');
            m_pendingKey.reset();
        }

        m_hasValue = true;
    }

    void newLine()
    {
        if (!m_pretty) return; // not pretty
        if (m_depth == 0 && !m_hasValue) return; // no new line for initial value

        auto& out = *m_out.rdbuf();
        out.sputc('\n');
        static constexpr std::string_view indent = "  ";
        for (uint32_t i = 0; i < m_depth; ++i)
        {
            out.sputn(indent.data(), indent.size());
        }
    }

    std::ostream& openStringStream()
    {
        prepareWriteVal();
        HUSE_ASSERT_INTERNAL(!m_stringStream);
        m_out.rdbuf()->sputc('"');
        m_stringStream = new (&m_stringStreamBuffer) JsonOStream(m_out);
        return m_stringStream->stream;
    }

    void closeStringStream()
    {
        assert(!!m_stringStream);
        m_stringStream->~JsonOStream();
        m_stringStream = nullptr;
        m_out.rdbuf()->sputc('"');
    }

    void throwException(const std::string& msg) const
    {
        throw SerializerException(msg);
    }

    std::optional<std::string_view> m_pendingKey;
    bool m_hasValue = false; // used to check whether a coma is needed
    const bool m_pretty;
    uint32_t m_depth = 0; // used to indent if pretty

    union {
        JsonOStream m_stringStreamBuffer;
    };
    JsonOStream* m_stringStream = nullptr;
};

DYNAMIX_DEFINE_MIXIN(Domain, JsonSerializer)
    .implements<husePolySerialize_bool>()
    .implements<husePolySerialize_short>()
    .implements<husePolySerialize_ushort>()
    .implements<husePolySerialize_int>()
    .implements<husePolySerialize_uint>()
    .implements<husePolySerialize_long>()
    .implements<husePolySerialize_ulong>()
    .implements<husePolySerialize_llong>()
    .implements<husePolySerialize_ullong>()
    .implements<husePolySerialize_float>()
    .implements<husePolySerialize_double>()
    .implements<husePolySerialize_sv>()
    .implements<husePolySerialize_null>()
    .implements<husePolySerialize_discard>()
    .implements_by<openStringStream_msg>([](JsonSerializer* s) -> std::ostream& {
        return s->openStringStream();
    })
    .implements_by<closeStringStream_msg>([](JsonSerializer* s) {
        s->closeStringStream();
    })
    .implements_by<pushKey_msg>([](JsonSerializer* s, std::string_view key) {
        s->pushKey(key);
    })
    .implements_by<openObject_msg>([](JsonSerializer* s) {
        s->openObject();
    })
    .implements_by<closeObject_msg>([](JsonSerializer* s) {
        s->closeObject();
    })
    .implements_by<openArray_msg>([](JsonSerializer* s) {
        s->openArray();
    })
    .implements_by<closeArray_msg>([](JsonSerializer* s) {
        s->closeArray();
    })
    .implements_by<throwSerializerException_msg>([](const JsonSerializer* s, const std::string& str) {
        s->throwException(str);
    })
;

//void Serializer::do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) {
//    new (new_mixin) JsonSerializer(out, pretty);
//}

Serializer Make_Serializer(std::ostream& out, bool pretty) {
    Serializer ret;
    mutate(ret, dynamix::add<JsonSerializer>(out, pretty));
    return ret;
}

}
