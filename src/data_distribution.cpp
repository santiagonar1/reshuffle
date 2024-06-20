#include "data_distribution.hpp"

namespace reshuffle {
    auto make_block_wise(int num_values, int num_blocks) -> BlockCyclic {
        const int block_size = std::ceil(static_cast<double>(num_values) / num_blocks);
        return BlockCyclic(block_size, num_values);
    }
}// namespace reshuffle