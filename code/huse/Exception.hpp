// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "API.h"

#include <stdexcept>

namespace huse
{
class HUSE_API Exception : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
    virtual ~Exception() = 0; // still need to export the vtable
};

class HUSE_API SerializerException : public Exception
{
public:
    using Exception::Exception;
    ~SerializerException();
};

class HUSE_API DeserializerException : public Exception
{
public:
    using Exception::Exception;
    ~DeserializerException();
};

}
