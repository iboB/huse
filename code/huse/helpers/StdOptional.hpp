#pragma once
#include "../SerializerNode.hpp"
#include "../DeserializerNode.hpp"
#include <optional>

namespace huse {

template <typename S, typename T>
void huseSerialize(SerializerNode<S>& n, const std::optional<T>& opt) {
    if (!opt) {
        n.val(std::nullopt);
    }
    else {
        n.val(*opt);
    }
}

template <typename D, typename T>
void huseDeserialize(DeserializerNode<D>& n, std::optional<T>& opt) {
    if (!n) {
        opt = {};
    }
    else {
        n.val(opt.emplace());
    }
}

}
