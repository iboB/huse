// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "DeserializerInterface.hpp"
#include "DefineMsg.hpp"
#include "DeserializerObj.hpp"

namespace huse {
HUSE_DEFINE_D_MSG(bool, bool);
HUSE_DEFINE_D_MSG(short, short);
HUSE_DEFINE_D_MSG(unsigned short, ushort);
HUSE_DEFINE_D_MSG(int, int);
HUSE_DEFINE_D_MSG(unsigned int, uint);
HUSE_DEFINE_D_MSG(long, long);
HUSE_DEFINE_D_MSG(unsigned long, ulong);
HUSE_DEFINE_D_MSG(long long, llong);
HUSE_DEFINE_D_MSG(unsigned long long, ullong);
HUSE_DEFINE_D_MSG(float, float);
HUSE_DEFINE_D_MSG(double, double);
HUSE_DEFINE_D_MSG(std::string_view, sv);
HUSE_DEFINE_D_MSG(std::string, string);

DYNAMIX_DEFINE_SIMPLE_MSG(skip_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(loadStringStream_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(unloadStringStream_msg, unicast);

DYNAMIX_DEFINE_SIMPLE_MSG(loadObject_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(unloadObject_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(loadArray_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(unloadArray_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(curLength_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(loadKey_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(tryLoadKey_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(loadIndex_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(hasPending_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(pendingType_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(pendingKey_msg, unicast);
DYNAMIX_DEFINE_SIMPLE_MSG(optPendingKey_msg, unicast);

DYNAMIX_DEFINE_SIMPLE_MSG(throwDeserializerException_msg, unicast);
}
