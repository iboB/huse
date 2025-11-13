// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include "Exception.hpp"
#include "CtxObj.hpp"

// used to export vtables

namespace huse
{
Serializer::Serializer(const CtxObj* ctx)
    : ctx(ctx ? *ctx : CtxObj::defaultCtx)
{}

Serializer::~Serializer() = default;
void Serializer::throwException(const std::string& msg) {
    throw SerializerException(msg);
}

Deserializer::~Deserializer() = default;

Exception::~Exception() = default;
SerializerException::~SerializerException() = default;
DeserializerException::~DeserializerException() = default;

}
