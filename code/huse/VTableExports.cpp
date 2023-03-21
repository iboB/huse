// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include "Exception.hpp"
#include "Domain.hpp"

// used to export vtables

namespace huse
{
Serializer::Serializer()
    : dynamix::object(dynamix::g::get_domain<SerializerDomain>())
{}

Deserializer::Deserializer()
    : dynamix::object(dynamix::g::get_domain<DeserializerDomain>())
{}

Exception::~Exception() = default;
SerializerException::~SerializerException() = default;
DeserializerException::~DeserializerException() = default;

}
