// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace huse
{

template <typename S>
class SerializerNode;
template <typename S>
class SerializerArray;
template <typename S>
class SerializerObject;
template <typename Serializer>
class SerializerSStream;

template <typename D>
class DeserializerNode;
template <typename D>
class DeserializerArray;
template <typename D>
class DeserializerObject;
class DeserializerSStream;
}
