// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once

namespace huse {

class ISerState;

template <typename Ser = ISerState>
class SerNode;
template <typename Ser = ISerState>
class SerArray;
template <typename Ser = ISerState>
class SerObject;

template <typename D>
class DeNode;
template <typename D>
class DeArray;
template <typename D>
class DeObject;

} // namespace huse
