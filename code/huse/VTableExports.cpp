// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "Serializer.hpp"
#include "Deserializer.hpp"
#include "SerializerException.hpp"

// used to export vtables

namespace huse
{
BasicSerializer::~BasicSerializer() = default;
BasicSerializer::Exception::~Exception() = default;
BasicDeserializer::~BasicDeserializer() = default;
}
