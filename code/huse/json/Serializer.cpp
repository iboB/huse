// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Serializer.hpp"

#include "../Exception.hpp"

#include <msstl/charconv.hpp>

#include <cmath>
#include <exception>
#include <new>

namespace huse::json
{

Serializer::Serializer(std::ostream& out, bool pretty, Context ctx)
    : BasicSerializer(ctx)
    , m_out(out)
    , m_pretty(pretty)
{}

Serializer::~Serializer()
{
    if (std::uncaught_exceptions()) return; // nothing smart to do
    HUSE_ASSERT_INTERNAL(m_depth == 0);
}

void Serializer::writeRawJson(std::string_view key, std::string_view json)
{
    pushKey(key);
    prepareWriteVal();
    m_out << json;
    m_hasValue = true;
}

void Serializer::writeRawString(std::string_view str)
{
    prepareWriteVal();
    m_out.rdbuf()->sputn(str.data(), str.size());
}

template <typename T>
void Serializer::writeSmallInteger(T n)
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

void Serializer::write(bool val)
{
    static constexpr std::string_view t = "true", f = "false";
    writeRawString(val ? t : f);
}

void Serializer::write(nullptr_t) { writeRawString("null"); }

void Serializer::write(short val) { writeSmallInteger(val); }
void Serializer::write(unsigned short val) { writeSmallInteger(val); }
void Serializer::write(int val) { writeSmallInteger(val); }
void Serializer::write(unsigned int val) { writeSmallInteger(val); }

template <typename T>
void Serializer::writePotentiallyBigIntegerValue(T val)
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
void Serializer::write(long val) { writePotentiallyBigIntegerValue(val); }
void Serializer::write(unsigned long val) { writePotentiallyBigIntegerValue(val); }
void Serializer::write(long long val) { writePotentiallyBigIntegerValue(val); }
void Serializer::write(unsigned long long val) { writePotentiallyBigIntegerValue(val); }

template <typename T>
void Serializer::writeFloatValue(T val)
{
    if (std::isfinite(val))
    {
        char out[25]; // max length of double
        auto result = msstl::to_chars(out, out + sizeof(out), val);
        writeRawString(std::string_view(out, result.ptr - out));
    }
    else
    {
        throwException("Floating point value is not finite. Not supported by JSON");
    }
}

void Serializer::write(float val) { writeFloatValue(val); }
void Serializer::write(double val) { writeFloatValue(val); }

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
        "\\u0018","\\u0019","\\u001a","\\u001b","\\u001c","\\u001d","\\u001e","\\u001f" };
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
}

void Serializer::writeQuotedEscapedUTF8String(std::string_view str)
{
    m_out.put('"');
    writeEscapedUTF8StringToStreambuf(*m_out.rdbuf(), str);
    m_out.put('"');
}

void Serializer::write(std::string_view val)
{
    prepareWriteVal();
    writeQuotedEscapedUTF8String(val);
}

void Serializer::write(std::nullopt_t)
{
    m_pendingKey.reset();
}

void Serializer::pushKey(std::string_view k)
{
    HUSE_ASSERT_INTERNAL(!m_pendingKey);
    m_pendingKey = k;
}

void Serializer::open(char o)
{
    prepareWriteVal();
    m_out << o;
    m_hasValue = false;
    ++m_depth;
}

void Serializer::close(char c)
{
    HUSE_ASSERT_INTERNAL(m_depth);
    --m_depth;
    if (m_hasValue) newLine();
    m_out << c;
    m_hasValue = true;
}

void Serializer::openObject() { open('{'); }
void Serializer::closeObject() { close('}'); }
void Serializer::openArray() { open('['); }
void Serializer::closeArray() { close(']'); }

void Serializer::prepareWriteVal()
{
    if (m_hasValue)
    {
        m_out << ',';
    }

    newLine();

    if (m_pendingKey)
    {
        writeQuotedEscapedUTF8String(*m_pendingKey);
        m_out << ':';
        m_pendingKey.reset();
    }

    m_hasValue = true;
}

void Serializer::newLine()
{
    if (!m_pretty) return; // not pretty
    if (m_depth == 0 && !m_hasValue) return; // no new line for initial value
    m_out << '\n';
    static constexpr std::string_view indent = "  ";
    for (uint32_t i = 0; i < m_depth; ++i)
    {
        m_out << indent;
    }
}

namespace
{
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
            m_redirectTarget.sputc(ch);
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
}

class Serializer::JsonOStream
{
public:
    JsonOStream(std::ostream& rt)
        : streambuf(*rt.rdbuf())
        , stream(&streambuf)
    {}

    JsonRedirectStreambuf streambuf;
    std::ostream stream;
};

std::ostream& Serializer::openStringStream()
{
    prepareWriteVal();
    static_assert(sizeof(JsonOStream) == sizeof(m_stringStreamBuffer));
    HUSE_ASSERT_INTERNAL(!m_stringStream);
    m_out.put('"');
    m_stringStream = new (&m_stringStreamBuffer) JsonOStream(m_out);
    return m_stringStream->stream;
}

void Serializer::closeStringStream()
{
    assert(!!m_stringStream);
    m_stringStream->~JsonOStream();
    m_stringStream = nullptr;
    m_out.put('"');
}

}
