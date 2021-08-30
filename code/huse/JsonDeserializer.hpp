// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once
#include "Deserializer.hpp"

#include <cstddef>
#include <memory>

namespace huse
{
namespace sajson { class document; }

class HUSE_API JsonDeserializer final : public Deserializer
{
public:
    static JsonDeserializer fromConstString(std::string_view str);
    static JsonDeserializer fromMutableString(char* str, size_t size = size_t(-1));

    ~JsonDeserializer();

    [[noreturn]] virtual void throwException(std::string msg) const override;

private:
    JsonDeserializer(sajson::document&& sajsonDoc);

    virtual void read(bool& val) override;
    virtual void read(short& val) override;
    virtual void read(unsigned short& val) override;
    virtual void read(int& val) override;
    virtual void read(unsigned int& val) override;
    virtual void read(long& val) override;
    virtual void read(unsigned long& val) override;
    virtual void read(long long& val) override;
    virtual void read(unsigned long long& val) override;
    virtual void read(float& val) override;
    virtual void read(double& val) override;
    virtual void read(std::string_view& val) override;
    virtual void read(std::string& val) override;

    virtual void loadObject() override;
    virtual void unloadObject() override;

    virtual void loadArray() override;
    virtual void unloadArray() override;

    virtual int curLength() const override;

    virtual void loadKey(std::string_view key) override;
    virtual bool tryLoadKey(std::string_view key) override;
    virtual void loadIndex(int index) override;
    virtual std::optional<std::string_view> loadNextKey() override;

    virtual Type pendingType() const override;

    struct Impl; // hiding sajson from the outside world
    std::unique_ptr<Impl> m_i;
};

}
