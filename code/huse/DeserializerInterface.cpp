// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "DeserializerInterface.hpp"
#include "DefineMsg.hpp"
#include "DeserializerObj.hpp"
#include "Exception.hpp"

namespace huse {
HUSE_DEFINE_D_MSG(bool&, bool);
HUSE_DEFINE_D_MSG(short&, short);
HUSE_DEFINE_D_MSG(unsigned short&, ushort);
HUSE_DEFINE_D_MSG(int&, int);
HUSE_DEFINE_D_MSG(unsigned int&, uint);
HUSE_DEFINE_D_MSG(long&, long);
HUSE_DEFINE_D_MSG(unsigned long&, ulong);
HUSE_DEFINE_D_MSG(long long&, llong);
HUSE_DEFINE_D_MSG(unsigned long long&, ullong);
HUSE_DEFINE_D_MSG(float&, float);
HUSE_DEFINE_D_MSG(double&, double);
HUSE_DEFINE_D_MSG(std::string_view&, sv);
HUSE_DEFINE_D_MSG(std::string&, string);

HUSE_DEFINE_D_MSG(std::nullptr_t, null);
HUSE_DEFINE_D_MSG(std::nullopt_t, discard);

DYNAMIX_DEFINE_SIMPLE_MSG_EX(skip_msg, unicast, false, nullptr);

DYNAMIX_DEFINE_SIMPLE_MSG_EX(loadObject_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(unloadObject_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(loadArray_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(unloadArray_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(curLength_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(loadKey_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(tryLoadKey_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(loadIndex_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(hasPending_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(pendingType_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(pendingKey_msg, unicast, false, nullptr);
DYNAMIX_DEFINE_SIMPLE_MSG_EX(optPendingKey_msg, unicast, false, nullptr);

void throwDeserializerExceptionDefault(const Deserializer&, const std::string& msg) {
    throw DeserializerException(msg);
}
DYNAMIX_DEFINE_SIMPLE_MSG_EX(throwDeserializerException_msg, unicast, true, throwDeserializerExceptionDefault);
}
