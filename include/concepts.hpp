#ifndef RESHUFFLE_CONCEPTS_HPP
#define RESHUFFLE_CONCEPTS_HPP

namespace reshuffle {
    template<typename T>
    concept Iterable = std::ranges::range<T>;

    template<typename T>
    concept ContiguousContainer = std::ranges::contiguous_range<T>;

    template<typename T>
    concept FundamentalType = std::is_fundamental_v<T>;
}

#endif //RESHUFFLE_CONCEPTS_HPP
