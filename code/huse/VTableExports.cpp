// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Context.hpp"
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include "Exception.hpp"

// used to export vtables

namespace huse
{
Context::~Context() = default;
BasicSerializer::~BasicSerializer() = default;
BasicDeserializer::~BasicDeserializer() = default;
Exception::~Exception() = default;
SerializerException::~SerializerException() = default;
DeserializerException::~DeserializerException() = default;

void BasicSerializer::throwException(const std::string& msg) const
{
    throw SerializerException(msg);
}

void BasicDeserializer::throwException(const std::string& msg) const
{
    throw DeserializerException(msg);
}

}
