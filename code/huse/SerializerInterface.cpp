// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "SerializerInterface.hpp"
#include "DefineMsg.hpp"
#include "SerializerObj.hpp"
#include "Exception.hpp"

namespace huse {
HUSE_DEFINE_S_MSG(bool, bool);
HUSE_DEFINE_S_MSG(short, short);
HUSE_DEFINE_S_MSG(unsigned short, ushort);
HUSE_DEFINE_S_MSG(int, int);
HUSE_DEFINE_S_MSG(unsigned int, uint);
HUSE_DEFINE_S_MSG(long, long);
HUSE_DEFINE_S_MSG(unsigned long, ulong);
HUSE_DEFINE_S_MSG(long long, llong);
HUSE_DEFINE_S_MSG(unsigned long long, ullong);
HUSE_DEFINE_S_MSG(float, float);
HUSE_DEFINE_S_MSG(double, double);
HUSE_DEFINE_S_MSG(std::string_view, sv);
HUSE_DEFINE_S_MSG(std::nullptr_t, null);
HUSE_DEFINE_S_MSG(std::nullopt_t, discard);

DYNAMIX_DEFINE_SIMPLE_MSG_EX(openStringStream_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(closeStringStream_msg, unicast, false, nullptr);

DYNAMIX_DEFINE_SIMPLE_MSG_EX(pushKey_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(openObject_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(closeObject_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(openArray_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(closeArray_msg, unicast, false, nullptr);

void throwSerializerExceptionDefault(const Serializer&, const std::string& msg) {
    throw SerializerException(msg);
}
DYNAMIX_DEFINE_SIMPLE_MSG_EX(throwSerializerException_msg, unicast, true, throwSerializerExceptionDefault);
}
