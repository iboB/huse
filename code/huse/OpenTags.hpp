#pragma once

namespace huse {

template <typename Serializer>
class SerializerArray;
struct Array {
    template <typename Serializer>
    using SerializerNode = SerializerArray<Serializer>;
};

template <typename Serializer>
class SerializerObject;
struct Object {
    template <typename Serializer>
    using SerializerNode = SerializerObject<Serializer>;
};

} // namespace huse
