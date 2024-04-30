#ifndef RESHUFFLE_CONCEPTS_HPP
#define RESHUFFLE_CONCEPTS_HPP

#include <zpp_bits.h>

namespace reshuffle::concepts {
    template<typename T>
    concept DefaultConstructible = std::is_default_constructible_v<T>;

    template<typename T>
    concept TriviallySerializable = zpp::bits::concepts::byte_serializable<T>;

    template<typename T>
    concept NonTriviallySerializable = zpp::bits::concepts::has_serialize<T> && requires(T &t) {
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

    template <typename T>
    concept Matrix2D = requires(T t) {
        t[0][0];
    };
}

#endif //RESHUFFLE_CONCEPTS_HPP
