// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "StatefulSerializer.hpp"
#include "Exception.hpp"

// used to export vtables

namespace huse {
StatefulSerializer::~StatefulSerializer() = default;
void StatefulSerializer::throwException(const std::string& msg) {
    throw SerializerException(msg);
}

Exception::~Exception() = default;
SerializerException::~SerializerException() = default;
DeserializerException::~DeserializerException() = default;
}
