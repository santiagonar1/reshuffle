#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP


#include <type_traits>
#include <zpp_bits.h>

namespace reshuffle::concepts {
    template<typename T>
    concept Aggregate = std::is_aggregate_v<T>;

    template<typename T>
    concept Serializable = Aggregate<T> || zpp::bits::concepts::has_serialize<T>;

    template<typename T>
    concept FundamentalType = std::is_fundamental_v<T>;

    template<typename T>
    concept NeedsSerialization = !FundamentalType<T>;
}// namespace reshuffle::concepts

#endif//CONCEPTS_HPP
