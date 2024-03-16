#ifndef RESHUFFLE_CONCEPTS_HPP
#define RESHUFFLE_CONCEPTS_HPP

#include <zpp_bits.h>

namespace reshuffle {
    template<typename T>
    concept DefaultConstructible = std::is_default_constructible_v<T>;

    template<typename T>
    concept TriviallySerializable = zpp::bits::concepts::byte_serializable<T>;

    template<typename T>
    concept NonTriviallySerializable = zpp::bits::concepts::has_serialize<T> && requires(T &t) {
        // TODO: We only require this if class is not default_constructible. Add a constraint later on if needed
        { T::create() } -> std::same_as<T>;
    };

    template<typename T>
    concept Serializable = TriviallySerializable<T> || NonTriviallySerializable<T>;

    template<typename T>
    concept Iterable = std::ranges::range<T>;

    template<typename T>
    concept ContiguousContainer = std::ranges::contiguous_range<T>;

    template<typename T>
    concept FundamentalType = std::is_fundamental_v<T>;
}

#endif //RESHUFFLE_CONCEPTS_HPP
