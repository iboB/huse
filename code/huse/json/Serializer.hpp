// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../Serializer.hpp"

#include <type_traits>
#include <ostream>

namespace huse::json
{

class HUSE_API Serializer final : public BasicSerializer
{
public:
    explicit Serializer(std::ostream& out, bool pretty = false, Context* ctx = {});
    ~Serializer();

    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;
    Serializer(Serializer&&) = delete;
    Serializer& operator=(Serializer&&) = delete;

    void writeRawJson(std::string_view key, std::string_view json);

    // json imposed limits (max integer which can be stored in a double)
    static inline constexpr int64_t Max_Int64 = 9007199254740992ll;
    static inline constexpr int64_t Min_Int64 = -9007199254740992ll;
    static inline constexpr int64_t Max_Uint64 = 9007199254740992ull;

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

    virtual void write(std::nullptr_t) override;
    virtual void write(std::nullopt_t) override;

    virtual std::ostream& openStringStream() override;
    virtual void closeStringStream() override;

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
    void writeRawString(std::string_view str);
    template <typename T> void writeSmallInteger(T n);
    template <typename T> void writePotentiallyBigIntegerValue(T val);
    template <typename T> void writeFloatValue(T val);
    void writeQuotedEscapedUTF8String(std::string_view val); // this is used for keys, too

    // open close helpers
    void open(char o);
    void close(char c);

    void prepareWriteVal(); // called before each value print
    void newLine(); // prints new line and indentation if pretty otherwise does nothing

    // hacky pimpl, but that's the only way to skip allocations altogether with this class
    // the size of the aligned storage must be updated manually if JsonStream's definition gets updated
    std::aligned_storage_t<sizeof(std::ostream) + sizeof(std::streambuf) + sizeof(void*), alignof(void*)> m_stringStreamBuffer;
    class JsonOStream;
    JsonOStream* m_stringStream = nullptr;
};

}
