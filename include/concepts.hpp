#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP


#include <type_traits>
#include <zpp_bits.h>

#include "fixed_size.hpp"

namespace reshuffle::concepts {
    template<typename T>
    concept Aggregate = std::is_aggregate_v<T>;

    template<typename T>
    concept Serializable = Aggregate<T> || zpp::bits::concepts::has_serialize<T>;

    template<typename T>
    concept FundamentalType = std::is_fundamental_v<T>;

    template<typename T>
    concept MPIType = FundamentalType<T> || std::is_same_v<T, std::byte>;

    template<typename T>
    concept NeedsSerialization = !FundamentalType<T>;

    template<typename T>
    concept FixedSizeSerializable = internal::is_fixed_size_serializable_v<T>;
}// namespace reshuffle::concepts

#endif//CONCEPTS_HPP
