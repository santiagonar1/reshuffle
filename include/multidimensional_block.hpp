#ifndef MULTIDIMENSIONAL_BLOCK_HPP
#define MULTIDIMENSIONAL_BLOCK_HPP

#include "block.hpp"
#include "coordinates.hpp"

namespace reshuffle::dev {
    template<std::size_t N>
    using MultidimensionalBlock = std::array<Block, N>;

    template<std::size_t N>
    auto get_owner_coordinates(const MultidimensionalBlock<N> &multidimensional_block)
            -> reshuffle::internal::Coordinates<N> {
        auto owner_coordinates = reshuffle::internal::Coordinates<N>{};
        for (int i = 0; i < N; ++i) {
            owner_coordinates[i] = multidimensional_block[i].get_owner();
        }
        return owner_coordinates;
    }

    template<std::size_t N>
    auto get_num_elements(const MultidimensionalBlock<N> &multidimensional_block) -> int {
        auto num_elements = 1;
        for (int i = 0; i < N; ++i) {
            num_elements *= multidimensional_block[i].get_num_elements();
        }
        return num_elements;
    }
}// namespace reshuffle::dev

#endif//MULTIDIMENSIONAL_BLOCK_HPP
