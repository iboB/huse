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
    assert(m_depth == 0);
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
    if constexpr (sizeof(T) <= 4)
    {
        // gcc and clang have long equal intptr_t, msvc has long at 4 bytes
        writeSimpleValue(val);
    }
    else if constexpr (std::is_signed_v<T>)
    {
        // max integer which can be stored in a double
        constexpr int64_t MAX_D = 9007199254740992ll;
        constexpr int64_t MIN_D = -9007199254740992ll;

        if (val >= MIN_D && val <= MAX_D)
        {
            writeSimpleValue(val);
        }
        else
        {
            // throwException("integer too big");
        }
    }
    else
    {
        constexpr uint64_t MAX_D = 9007199254740992ull;

        if (val <= MAX_D)
        {
            writeSimpleValue(val);
        }
        else
        {
            // throwException("integer too big");
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
        // throwException("float not finite");
    }
}

void Serializer::write(float val) { writeFloatValue(val); }
void Serializer::write(double val) { writeFloatValue(val); }

void Serializer::writeEscapedUTF8String(std::string_view str)
{
    m_out.put('\"');

    for (auto c : str)
    {
        switch (c) {
            // http://www.json.org/
        case '"':
            m_out << "\\\"";
            break;
        case '\\':
            m_out << "\\\\";
            break;
        case '\n':
            m_out << "\\n";
            break;
        case '\r':
            m_out << "\\r";
            break;
        case '\b':
            m_out << "\\b";
            break;
        case '\t':
            m_out << "\\t";
            break;
        case '\f':
            m_out << "\\f";
            break;
        default:
        {
            if (c >= ' ')
            {
                m_out.put(c);
            }
            else
            {
                char num[3]; // max length of 8-bit integer in hex is 2, plus one byte for termination
                auto result = msstl::to_chars(num, num + sizeof(num), c, 16);
                *result.ptr = 0;
                const char* prefix = c < 16 ? "\\u000" : "\\u00";
                m_out << prefix << num;
            }
        }
        break;
        }
    }

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
    assert(!m_pendingKey);
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
    assert(m_depth);
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
