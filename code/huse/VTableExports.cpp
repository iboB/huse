// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Deserializer.hpp"
#include "Serializer.hpp"
#include "Exception.hpp"

// used to export vtables

namespace huse {
Deserializer::~Deserializer() = default;

Serializer::~Serializer() = default;
void Serializer::throwException(const std::string& msg) {
    throw SerializerException(msg);
}

Exception::~Exception() = default;
SerializerException::~SerializerException() = default;
DeserializerException::~DeserializerException() = default;
}
