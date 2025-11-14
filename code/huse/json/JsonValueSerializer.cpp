// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "JsonValueSerializer.hpp"
#include "Serializer.hpp"
#include "Limits.hpp"

#include "../Exception.hpp"
#include "../impl/Assert.hpp"
#include "../impl/Charconv.hpp"

#include <dynamix/define_mixin.hpp>
#include <dynamix/mutate.hpp>

#include <cmath>
#include <exception>
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

void writeQuotedEscapedUTF8StringToStream(std::ostream& sout, std::string_view str) {
    auto& out = *sout.rdbuf();
    out.sputc('"');
    writeEscapedUTF8StringToStreambuf(out, str);
    out.sputc('"');
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

} // namespace

JsonValueSerializer::JsonValueSerializer(std::ostream& out, bool pretty)
    : m_out(out)
    , m_pretty(pretty)
{}

JsonValueSerializer::~JsonValueSerializer() {
    if (std::uncaught_exceptions()) return; // nothing smart to do
    HUSE_ASSERT_INTERNAL(m_depth == 0);
}


void JsonValueSerializer::writeRawJson(std::string_view json) {
    prepareWriteVal();
    m_out.rdbuf()->sputn(json.data(), json.size());
}

void JsonValueSerializer::writeValue(bool val) {
    static constexpr std::string_view t = "true", f = "false";
    writeRawJson(val ? t : f);
}

void JsonValueSerializer::writeValue(std::nullptr_t) { writeRawJson("null"); }

template <typename T>
void JsonValueSerializer::writeSmallInteger(T n) {
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

void JsonValueSerializer::writeValue(short val) { writeSmallInteger(val); }
void JsonValueSerializer::writeValue(unsigned short val) { writeSmallInteger(val); }
void JsonValueSerializer::writeValue(int val) { writeSmallInteger(val); }
void JsonValueSerializer::writeValue(unsigned int val) { writeSmallInteger(val); }

template <typename T>
void JsonValueSerializer::writePotentiallyBigIntegerValue(T val) {
    static constexpr std::string_view IntegerTooBig = "Integer value is bigger than maximum allowed for JSON";

    if constexpr (sizeof(T) <= 4) {
        // gcc and clang have long equal intptr_t, msvc has long at 4 bytes
        writeSmallInteger(val);
    }
    else if constexpr (std::is_signed_v<T>) {
        if (val >= Min_Int64 && val <= Max_Int64) {
            writeSmallInteger(val);
        }
        else {
            throwException(std::string(IntegerTooBig));
        }
    }
    else {
        if (val <= Max_Uint64) {
            writeSmallInteger(val);
        }
        else {
            throwException(std::string(IntegerTooBig));
        }
    }
}

// some values may not fit json's numbers
void JsonValueSerializer::writeValue(long val) { writePotentiallyBigIntegerValue(val); }
void JsonValueSerializer::writeValue(unsigned long val) { writePotentiallyBigIntegerValue(val); }
void JsonValueSerializer::writeValue(long long val) { writePotentiallyBigIntegerValue(val); }
void JsonValueSerializer::writeValue(unsigned long long val) { writePotentiallyBigIntegerValue(val); }

template <typename T>
void JsonValueSerializer::writeFloatValue(T val) {
    if (std::isfinite(val)) {
        char out[25]; // max length of double
        auto result = HUSE_CHARCONV_NAMESPACE::to_chars(out, out + sizeof(out), val);
        writeRawJson(std::string_view(out, result.ptr - out));
    }
    else {
        throwException("Floating point value is not finite. Not supported by JSON");
    }
}

void JsonValueSerializer::writeValue(float val) { writeFloatValue(val); }
void JsonValueSerializer::writeValue(double val) { writeFloatValue( val); }

void JsonValueSerializer::writeValue(std::string_view val) {
    prepareWriteVal();
    writeQuotedEscapedUTF8StringToStream(m_out, val);
}

void JsonValueSerializer::writeValue(std::nullopt_t) {
    m_pendingKey.reset();
}

void JsonValueSerializer::pushKey(std::string_view k) {
    HUSE_ASSERT_INTERNAL(!m_pendingKey);
    m_pendingKey = k;
}

void JsonValueSerializer::newLine() {
    if (!m_pretty) return; // not pretty
    if (m_depth == 0 && !m_hasValue) return; // no new line for initial value

    auto& out = *m_out.rdbuf();
    out.sputc('\n');
    static constexpr std::string_view indent = "  ";
    for (uint32_t i = 0; i < m_depth; ++i) {
        out.sputn(indent.data(), indent.size());
    }
}

void JsonValueSerializer::prepareWriteVal() {
    auto& out = *m_out.rdbuf();

    if (m_hasValue) {
        out.sputc(',');
    }

    newLine();

    if (m_pendingKey) {
        writeQuotedEscapedUTF8StringToStream(m_out, *m_pendingKey);
        out.sputc(':');
        m_pendingKey.reset();
    }

    m_hasValue = true;
}

void JsonValueSerializer::open(char o) {
    prepareWriteVal();
    m_out.rdbuf()->sputc(o);
    m_hasValue = false;
    ++m_depth;
}

void JsonValueSerializer::close(char c) {
    HUSE_ASSERT_INTERNAL(m_depth);
    --m_depth;
    if (m_hasValue) newLine();
    m_out.rdbuf()->sputc(c);
    m_hasValue = true;
}

void JsonValueSerializer::openObject() { open('{'); }
void JsonValueSerializer::closeObject() { close('}'); }
void JsonValueSerializer::openArray() { open('['); }
void JsonValueSerializer::closeArray() { close(']'); }


struct JsonValueSerializer::JsonOStream {
    JsonOStream(std::ostream& rt)
        : streambuf(*rt.rdbuf())
        , stream(&streambuf)
    {}

    JsonRedirectStreambuf streambuf;
    std::ostream stream;
};

std::ostream& JsonValueSerializer::openStringStream() {
    prepareWriteVal();
    m_out.rdbuf()->sputc('"');

    if (!m_stringStream) {
        m_stringStream = std::make_unique<std::optional<JsonOStream>>();
    }
    HUSE_ASSERT_INTERNAL(!*m_stringStream);
    m_stringStream->emplace(m_out);
    return (*m_stringStream)->stream;
}

void JsonValueSerializer::closeStringStream() {
    assert(!!m_stringStream && !!*m_stringStream);
    m_stringStream->reset();
    m_out.rdbuf()->sputc('"');
}

}
