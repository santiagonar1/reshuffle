#ifndef RESHUFFLE_DIMENSIONS_HPP
#define RESHUFFLE_DIMENSIONS_HPP

#include <array>
#include <numeric>

namespace reshuffle {
    template<std::size_t N>
    using Dimensions = std::array<int, N>;

    template<std::size_t N>
    auto calc_total_num_values(const Dimensions<N> &d) -> int {
        return std::accumulate(d.cbegin(), d.cend(), 1, std::multiplies());
    }
}// namespace reshuffle

#endif//RESHUFFLE_DIMENSIONS_HPP
