#pragma once
#include "Fwd.hpp"

namespace huse {

struct Array {
    template <typename Serializer>
    using SerializerNode = SerializerArray<Serializer>;
    template <typename Deserializer>
    using DeserializerNode = DeserializerArray<Deserializer>;
};

struct Object {
    template <typename Serializer>
    using SerializerNode = SerializerObject<Serializer>;
    template <typename Deserializer>
    using DeserializerNode = DeserializerObject<Deserializer>;
};

struct StringStream {
    template <typename Serializer>
    using SerializerNode = SerializerSStream<Serializer>;
    template <typename Deserializer>
    using DeserializerNode = DeserializerSStream;
};

} // namespace huse
