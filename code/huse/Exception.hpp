// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
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
};

class HUSE_API DeserializerException : public Exception
{
public:
    using Exception::Exception;
};

}
