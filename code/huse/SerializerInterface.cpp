// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "SerializerInterface.hpp"
#include "DefineMsg.hpp"
#include "Serializer2.hpp"

namespace huse {
HUSE_DEFINE_SERIALIZE_MSG(bool, bool);
HUSE_DEFINE_SERIALIZE_MSG(short, short);
HUSE_DEFINE_SERIALIZE_MSG(unsigned short, ushort);
HUSE_DEFINE_SERIALIZE_MSG(int, int);
HUSE_DEFINE_SERIALIZE_MSG(unsigned int, uint);
HUSE_DEFINE_SERIALIZE_MSG(long, long);
HUSE_DEFINE_SERIALIZE_MSG(unsigned long, ulong);
HUSE_DEFINE_SERIALIZE_MSG(long long, llong);
HUSE_DEFINE_SERIALIZE_MSG(unsigned long long, ullong);
HUSE_DEFINE_SERIALIZE_MSG(float, float);
HUSE_DEFINE_SERIALIZE_MSG(double, double);
HUSE_DEFINE_SERIALIZE_MSG(std::string_view, sv);
HUSE_DEFINE_SERIALIZE_MSG(std::nullptr_t, nullptr_t);
HUSE_DEFINE_SERIALIZE_MSG(std::nullopt_t, nullopt_t);

DYNAMIX_DEFINE_SIMPLE_MSG(openStringStream_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(closeStringStream_msg, unicast);

DYNAMIX_DEFINE_SIMPLE_MSG(openObject_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(closeObject_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(openArray_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(closeArray_msg, unicast);

DYNAMIX_DEFINE_SIMPLE_MSG(throwSerializerException_msg, unicast);
}
