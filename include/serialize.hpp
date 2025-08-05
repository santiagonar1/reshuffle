#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include "concepts.hpp"
#include "profiler.hpp"

namespace reshuffle::internal {

    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto serialize(std::span<const T> values) -> std::vector<std::byte> {
        PROFILE_SCOPE_NAMED("serialize");

        auto [bytes, out] = zpp::bits::data_out();

        std::ranges::for_each(values, [&out](const auto &p) { out(p).or_throw(); });

        return bytes;
    }

    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto serialize(const std::vector<T> &values) -> std::vector<std::byte> {
        return serialize(std::span{values});
    }

    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    auto serialize(const std::vector<T> &values, std::span<std::byte> save_here) -> void {
        PROFILE_SCOPE_NAMED("serialize");

        auto out = zpp::bits::out{save_here};
        std::ranges::for_each(values, [&out](const auto &p) { out(p).or_throw(); });
    }


    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto serialize(std::span<T> values) -> std::vector<std::byte> {
        return serialize(std::span<const T>(values));
    }

    // We define it this way to make sure we never call this with something that does not
    // require serialization (e.g., an integer), as this operation has an overhead
    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto deserialize(const std::vector<std::byte> &bytes) -> std::vector<T> {
        PROFILE_SCOPE_NAMED("deserialize");

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
    [[nodiscard]] auto deserialize(std::span<const std::byte> bytes) -> std::vector<T> {
        PROFILE_SCOPE_NAMED("deserialize");

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
