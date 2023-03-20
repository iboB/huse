// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "SerializerInterface.hpp"
#include "DefineMsg.hpp"

namespace huse {
HUSE_DEFINE_SERIALIZE_MSG(bool, bool);
HUSE_DEFINE_SERIALIZE_MSG(short, short);
HUSE_DEFINE_SERIALIZE_MSG(unsigned short, ushort);
}
