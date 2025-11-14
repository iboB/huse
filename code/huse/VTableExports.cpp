// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Serializer.hpp"
#include "DeserializerRoot.hpp"
#include "Exception.hpp"

// used to export vtables

namespace huse
{
Serializer::~Serializer() = default;
void Serializer::throwException(const std::string& msg) {
    throw SerializerException(msg);
}

Deserializer::~Deserializer() = default;

Exception::~Exception() = default;
SerializerException::~SerializerException() = default;
DeserializerException::~DeserializerException() = default;

}
