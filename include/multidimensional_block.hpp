#ifndef MULTIDIMENSIONAL_BLOCK_HPP
#define MULTIDIMENSIONAL_BLOCK_HPP

#include "block.hpp"

namespace reshuffle::dev {
    template<std::size_t N>
    using MultidimensionalBlock = std::array<Block, N>;

    template<std::size_t N>
    auto get_owner_coordinates(const MultidimensionalBlock<N> &multidimensional_block)
            -> reshuffle::internal::Coordinates<N> {
        auto owner_coordinates = reshuffle::internal::Coordinates<N>{};
        auto reversed_multidimensional_block = multidimensional_block | std::views::reverse;
        for (int i = 0; i < N; ++i) {
            owner_coordinates[i] = reversed_multidimensional_block[i].get_owner();
        }
        return owner_coordinates;
    }
}// namespace reshuffle::dev

#endif//MULTIDIMENSIONAL_BLOCK_HPP
