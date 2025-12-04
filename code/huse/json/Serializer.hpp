// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../Serializer.hpp"
#include <iosfwd>
#include <memory>

namespace huse::json {

class HUSE_API JsonSerializer : virtual public Serializer {
public:
    JsonSerializer(std::ostream& out, bool pretty = false);
    ~JsonSerializer();

    virtual void writeValue(bool val) final override;
    virtual void writeValue(short val) final override;
    virtual void writeValue(unsigned short val) final override;
    virtual void writeValue(int val) final override;
    virtual void writeValue(unsigned int val) final override;
    virtual void writeValue(long val) final override;
    virtual void writeValue(unsigned long val) final override;
    virtual void writeValue(long long val) final override;
    virtual void writeValue(unsigned long long val) final override;
    virtual void writeValue(float val) final override;
    virtual void writeValue(double val) final override;
    virtual void writeValue(std::string_view val) final override;
    virtual void writeValue(std::nullptr_t) final override;
    virtual void writeValue(std::nullopt_t) final override;
    using Serializer::writeValue;

    virtual std::ostream& openStringStream() final override;
    virtual void closeStringStream() final override;

    virtual void pushKey(std::string_view key) final override;

    virtual void openObject() final override;
    virtual void closeObject() final override;
    virtual void openArray() final override;
    virtual void closeArray() final override;

    using Serializer::open;

    void writeRawJson(std::string_view json);
    std::ostream& out() { return m_out; }

private:
    void newLine();
    void prepareWriteVal();
    void open(char o);
    void close(char c);

    template <typename T> void writeSmallInteger(T n);
    template <typename T> void writePotentiallyBigIntegerValue(T val);
    template <typename T> void writeFloatValue(T val);

    std::ostream& m_out;
    std::optional<std::string_view> m_pendingKey;
    bool m_hasValue = false; // used to check whether a coma is needed
    const bool m_pretty;
    uint32_t m_depth = 0; // used to indent if pretty

    struct JsonOStream;
    std::unique_ptr<std::optional<JsonOStream>> m_stringStream;
};

} // namespace huse::json
