// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "Serializer.hpp"

#include <msstl/charconv.hpp>

#include <cmath>
#include <iostream>
#include <type_traits>
#include <exception>

namespace huse::json
{

Serializer::Serializer(std::ostream& out, bool pretty)
    : m_out(out)
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

template <typename T>
void Serializer::writeSimpleValue(T val)
{
    prepareWriteVal();
    m_out << val;
}

void Serializer::write(bool val)
{
    static constexpr std::string_view t = "true", f = "false";
    writeSimpleValue(val ? t : f);
}

void Serializer::write(nullptr_t) { writeSimpleValue("null"); }

void Serializer::write(short val) { writeSimpleValue(val); }
void Serializer::write(unsigned short val) { writeSimpleValue(val); }
void Serializer::write(int val) { writeSimpleValue(val); }
void Serializer::write(unsigned int val) { writeSimpleValue(val); }

template <typename T>
void Serializer::writePotentiallyBigIntegerValue(T val)
{
    static const std::string_view IntegerTooBig = "Integer value is bigger than maximum allowed for JSON";

    if constexpr (sizeof(T) <= 4)
    {
        // gcc and clang have long equal intptr_t, msvc has long at 4 bytes
        writeSimpleValue(val);
    }
    else if constexpr (std::is_signed_v<T>)
    {
        if (val >= Min_Int64 && val <= Max_Int64)
        {
            writeSimpleValue(val);
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
            writeSimpleValue(val);
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
        writeSimpleValue(std::string_view(out, result.ptr - out));
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
        "\\u0018","\\u0019","\\u001a","\\u001b","\\u001c","\\u001d","\\u001e","\\u001f"};
    return belowSpace[u];
}
}

void Serializer::writeEscapedUTF8String(std::string_view str)
{
    m_out.put('\"');

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
            if (p != begin) m_out.write(begin, p-begin);
            m_out << *esc;
            begin = ++p;
        }
    }
    if (p != begin) m_out.write(begin, p-begin);

    m_out.put('\"');
}

void Serializer::write(std::string_view val)
{
    prepareWriteVal();
    writeEscapedUTF8String(val);
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
        writeEscapedUTF8String(*m_pendingKey);
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

}
