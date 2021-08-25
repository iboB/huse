// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once
#include "Serializer.hpp"

#include <iosfwd>

namespace huse
{

class HUSE_API JsonSerializer final : public Serializer
{
public:
    JsonSerializer(std::ostream& out, bool pretty = false);
    ~JsonSerializer();

    JsonSerializer(const JsonSerializer&) = delete;
    JsonSerializer& operator=(const JsonSerializer&) = delete;
    JsonSerializer(JsonSerializer&&) = delete;
    JsonSerializer& operator=(JsonSerializer&&) = delete;

    void writeRawJson(std::string_view key, std::string_view json);

private:
    virtual void write(bool val) override;
    virtual void write(short val) override;
    virtual void write(unsigned short val) override;
    virtual void write(int val) override;
    virtual void write(unsigned int val) override;
    virtual void write(long val) override;
    virtual void write(unsigned long val) override;
    virtual void write(long long val) override;
    virtual void write(unsigned long long val) override;
    virtual void write(float val) override;
    virtual void write(double val) override;
    virtual void write(std::string_view val) override;

    virtual void write(nullptr_t) override;
    virtual void write(std::nullopt_t) override;

    virtual void pushKey(std::string_view k) override;

    virtual void openObject() override;
    virtual void closeObject() override;

    virtual void openArray() override;
    virtual void closeArray() override;

    std::ostream& m_out;

    std::optional<std::string_view> m_pendingKey;
    bool m_hasValue = false; // used to check whether a coma is needed
    const bool m_pretty;
    uint32_t m_depth = 0; // used to indent if pretty


    // internal write helpers
    template <typename T> void writeSimpleValue(T val);
    template <typename T> void writePotentiallyBigIntegerValue(T val);
    template <typename T> void writeFloatValue(T val);
    void writeEscapedUTF8String(std::string_view val); // this is used for keys, too

    // open close helpers
    void open(char o);
    void close(char c);

    void prepareWriteVal(); // called before each value print
    void newLine(); // prints new line and indentation if pretty otherwise does nothing

    void throwException(std::string text) const;
};

}
