#ifndef FIXED_SIZE_DATA_HPP
#define FIXED_SIZE_DATA_HPP

#include <fixed_size.hpp>

struct FixedSizeData {
    int _dummy_int{};

    auto operator<=>(const FixedSizeData &) const = default;
};

RESHUFFLE_HAS_FIXED_SIZE_SERIALIZABLE(FixedSizeData);

#endif//FIXED_SIZE_DATA_HPP
