// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"
#include "DeclareMsg.hpp"

#include <string_view>
#include <optional>

namespace huse {
HUSE_SERIALIZE_MSG(HUSE_API, bool, bool);
HUSE_SERIALIZE_MSG(HUSE_API, short, short);
HUSE_SERIALIZE_MSG(HUSE_API, unsigned short, ushort);
HUSE_SERIALIZE_MSG(HUSE_API, int, int);
HUSE_SERIALIZE_MSG(HUSE_API, unsigned int, uint);
HUSE_SERIALIZE_MSG(HUSE_API, long, long);
HUSE_SERIALIZE_MSG(HUSE_API, unsigned long, ulong);
HUSE_SERIALIZE_MSG(HUSE_API, long long, llong);
HUSE_SERIALIZE_MSG(HUSE_API, unsigned long long, ullong);
HUSE_SERIALIZE_MSG(HUSE_API, float, float);
HUSE_SERIALIZE_MSG(HUSE_API, double, double);
HUSE_SERIALIZE_MSG(HUSE_API, std::string_view, sv);

HUSE_SERIALIZE_MSG(HUSE_API, std::nullptr_t, nullptr_t); // write null explicitly
HUSE_SERIALIZE_MSG(HUSE_API, std::nullopt_t, nullopt_t); // discard current value

// helper
inline void husePolySerialize(Serializer& s, const char* str) { husePolySerialize(s, std::string_view(str)); }

// private interface

DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, openStringStream_msg, std::ostream&(Serializer&));
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, closeStringStream_msg, void(Serializer&));

DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, openObject_msg, void(Serializer&));
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, closeObject_msg, void(Serializer&));
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, openArray_msg, void(Serializer&));
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, closeArray_msg, void(Serializer&));

// optional override
// has a default implementation
DYNAMIX_DECLARE_EXPORTED_SIMPLE_MSG(HUSE_API, throwSerializerException_msg, void(const Serializer&, const std::string&));
[[noreturn]] inline void throwSerializerException(const Serializer& s, const std::string& msg) {
    throwSerializerException_msg::call(s, msg);
}
}
