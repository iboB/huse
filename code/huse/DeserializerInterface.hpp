// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "DeclareMsg.hpp"
#include "Type.hpp"

#include <string_view>
#include <optional>
#include <iosfwd>
#include <string>

namespace huse {
HUSE_D_MSG(HUSE_API, bool&, bool);
HUSE_D_MSG(HUSE_API, short&, short);
HUSE_D_MSG(HUSE_API, unsigned short&, ushort);
HUSE_D_MSG(HUSE_API, int&, int);
HUSE_D_MSG(HUSE_API, unsigned int&, uint);
HUSE_D_MSG(HUSE_API, long&, long);
HUSE_D_MSG(HUSE_API, unsigned long&, ulong);
HUSE_D_MSG(HUSE_API, long long&, llong);
HUSE_D_MSG(HUSE_API, unsigned long long&, ullong);
HUSE_D_MSG(HUSE_API, float&, float);
HUSE_D_MSG(HUSE_API, double&, double);
HUSE_D_MSG(HUSE_API, std::string_view&, sv);
HUSE_D_MSG(HUSE_API, std::string&, string);

HUSE_D_MSG(HUSE_API, std::nullptr_t, null);
HUSE_D_MSG(HUSE_API, std::nullopt_t, discard);

// private interface

// skip a value
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, skip_msg, void(Deserializer&));

// stateful reads
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, loadStringStream_msg, std::istream&(Deserializer&));
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, unloadStringStream_msg, void(Deserializer&));

// implementation interface
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, loadObject_msg, void(Deserializer&));
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, unloadObject_msg, void(Deserializer&));
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, loadArray_msg, void(Deserializer&));
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, unloadArray_msg, void(Deserializer&));

// number of sub-nodes in current node
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, curLength_msg, int(const Deserializer&));

// throw if no key
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, loadKey_msg, void(Deserializer&, std::string_view key));

// load and resturn true if key exists, otherwise return false
// equivalent to (but more optimized than)
// if (hasKey(k)) { loadKey(k); return true; } else return false;
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, tryLoadKey_msg, bool(Deserializer&, std::string_view key));

// throw if no index
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, loadIndex_msg, void(Deserializer&, int index));

DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, hasPending_msg, bool(const Deserializer&));

DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, pendingType_msg, Type(const Deserializer&));

// throw if no pending key
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, pendingKey_msg, std::string_view(const Deserializer&));

// return pending key or nullopt if there is none
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, optPendingKey_msg, std::optional<std::string_view>(const Deserializer&));

// optional override
// has a default implementation (default impl throws with no context)
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, throwDeserializerException_msg, void(const Deserializer&, const std::string&));
[[noreturn]] inline void throwDeserializerException(const Deserializer& d, const std::string& msg) {
    throwDeserializerException_msg::call(d, msg);
}

}
