#ifndef RESHUFFLE_CONCEPTS_HPP
#define RESHUFFLE_CONCEPTS_HPP

namespace reshuffle::concepts {
    template<typename T>
    concept DefaultConstructible = std::is_default_constructible_v<T>;

    template<typename T>
    concept Iterable = std::ranges::range<T>;

    template<typename T>
    concept ContiguousContainer = std::ranges::contiguous_range<T>;

    template<typename T>
    concept FundamentalType = std::is_fundamental_v<T>;

    template<typename T>
    concept Matrix2D = requires(T t) { t[0][0]; };
}// namespace reshuffle::concepts

#endif//RESHUFFLE_CONCEPTS_HPP
