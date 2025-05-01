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
        // Note: the easies way to understand why we need to reverse the coordinates is with an
        // example. So far through the code we assume that MultidimensionalBlock stores the Blocks
        // like {X, Y} (i.e., first the horizontal, then the vertical). Nonetheless,
        // the processor grid assumes a row major ordering. Let's assume that the coordinates
        // stored in {X.get_owner(), Y.get_owner()} are {1, 0}, which, assuming a 2x2 division,
        // would locate the block in the top right corner. Now, this block should belong to
        // the processor with coordinates {0, 1}, with rank_id 1.
        auto reversed_multidimensional_block = multidimensional_block | std::views::reverse;
        for (int i = 0; i < N; ++i) {
            owner_coordinates[i] = reversed_multidimensional_block[i].get_owner();
        }
        return owner_coordinates;
    }
}// namespace reshuffle::dev

#endif//MULTIDIMENSIONAL_BLOCK_HPP
