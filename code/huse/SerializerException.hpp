// huse
// Copyright (c) 2021 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once
#include "API.h"
#include "Serializer.hpp"

#include <stdexcept>

namespace huse
{
class HUSE_API Serializer::Exception : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
    virtual ~Exception(); // still need to export the vtable
};
}
