#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include "concepts.hpp"

namespace reshuffle::internal {

    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    auto serialize(const std::vector<T> &values) -> std::vector<std::byte> {
        auto [bytes, out] = zpp::bits::data_out();

        std::ranges::for_each(values, [&out](const auto &p) { out(p).or_throw(); });

        return bytes;
    }

    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    auto serialize(std::span<const T> values) -> std::vector<std::byte> {
        auto [bytes, out] = zpp::bits::data_out();

        std::ranges::for_each(values, [&out](const auto &p) { out(p).or_throw(); });

        return bytes;
    }


    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    auto serialize(std::span<T> values) -> std::vector<std::byte> {
        return serialize(std::span<const T>(values));
    }

    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    auto deserialize(const std::vector<std::byte> &bytes) -> std::vector<T> {
        auto values = std::vector<T>();

        auto in = zpp::bits::in(bytes);
        while (in.position() < bytes.size()) {
            T p;
            in(p).or_throw();
            values.push_back(p);
        }

        return values;
    }

    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    auto deserialize(std::span<const std::byte> bytes) -> std::vector<T> {
        auto values = std::vector<T>();

        auto in = zpp::bits::in(bytes);
        while (in.position() < bytes.size()) {
            T p;
            in(p).or_throw();
            values.push_back(p);
        }

        return values;
    }
}// namespace reshuffle::internal

#endif//SERIALIZE_HPP
