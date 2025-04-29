#ifndef MULTIDIMENSIONAL_BLOCK_HPP
#define MULTIDIMENSIONAL_BLOCK_HPP

#include "block.hpp"

namespace reshuffle::dev {
    template<std::size_t N>
    using MultidimensionalBlock = std::array<Block, N>;
}

#endif//MULTIDIMENSIONAL_BLOCK_HPP
