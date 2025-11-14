// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "ValueSerializer.hpp"
#include "Exception.hpp"

// used to export vtables

namespace huse {
ValueSerializer::~ValueSerializer() = default;
void ValueSerializer::throwException(const std::string& msg) {
    throw SerializerException(msg);
}

Exception::~Exception() = default;
SerializerException::~SerializerException() = default;
DeserializerException::~DeserializerException() = default;
}
