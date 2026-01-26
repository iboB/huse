#pragma once
#include "Fwd.hpp"
#include <string_view>

namespace huse {

struct StringStream {
    template <typename Serializer>
    SerializerSStream<Serializer> huseOpen(SerializerNode<Serializer>& parent) {
        return SerializerSStream<Serializer>(parent, parent._s().openStringStream());
    }

    template <typename Deserializer>
    DeserializerSStream huseOpen(DeserializerNode<Deserializer>& parent) {
        std::string_view str;
        parent.val(str);
        return DeserializerSStream(str);
    }
};

} // namespace huse
