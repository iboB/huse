// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../API.h"
#include "../ValueSerializer.hpp"
#include <iosfwd>
#include <memory>

namespace huse::json {

class JsonValueSerializer : public ValueSerializer {
public:
    JsonValueSerializer(std::ostream& out, bool pretty = false);
    ~JsonValueSerializer();

    virtual void writeValue(bool val) override;
    virtual void writeValue(short val) override;
    virtual void writeValue(unsigned short val) override;
    virtual void writeValue(int val) override;
    virtual void writeValue(unsigned int val) override;
    virtual void writeValue(long val) override;
    virtual void writeValue(unsigned long val) override;
    virtual void writeValue(long long val) override;
    virtual void writeValue(unsigned long long val) override;
    virtual void writeValue(float val) override;
    virtual void writeValue(double val) override;
    virtual void writeValue(std::string_view val) override;
    virtual void writeValue(std::nullptr_t) override;
    virtual void writeValue(std::nullopt_t) override;

    virtual std::ostream& openStringStream() override;
    virtual void closeStringStream() override;

    virtual void pushKey(std::string_view key) override;

    virtual void openObject() override;
    virtual void closeObject() override;
    virtual void openArray() override;
    virtual void closeArray() override;

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
