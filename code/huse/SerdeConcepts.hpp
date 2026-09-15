// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include <concepts>

namespace huse::impl {

template <typename T, typename S>
concept HasStateSerde = requires(T t, S& state) {
    huseState_serde(state, t);
};
template <typename T, typename N>
concept HasSerdeStaticFunc = requires(T t, const N& node) {
    T::huse_serde(node, t);
};
template <typename T, typename N>
concept HasSerdeMethod = requires(T t, const N& node) {
    t.huse_serde(node);
};
template <typename T, typename N>
concept HasSerdeFreeFunc = requires(T t, const N& node) {
    huse_serde(node, t);
};

template <typename T, typename O>
concept HasSerdeFlatStaticFunc = requires(T t, const O& obj) {
    T::huse_serdeFlat(obj, t);
};
template <typename T, typename O>
concept HasSerdeFlatMethod = requires(T t, const O& obj) {
    t.huse_serdeFlat(obj);
};
template <typename T, typename O>
concept HasSerdeFlatFreeFunc = requires(T t, const O& obj) {
    huse_serdeFlat(obj, t);
};

} // namespace huse::impl
