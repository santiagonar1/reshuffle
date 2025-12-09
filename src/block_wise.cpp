#include "block_wise.hpp"

#include "block_cyclic.hpp"

namespace reshuffle::internal {
    auto create_evenly_blocks(const int num_values, const int num_processors)
            -> std::vector<Block> {
        const auto block_size = std::ceil(static_cast<double>(num_values) / num_processors);
        return create_blocks(num_values, block_size, num_processors);
    }
}// namespace reshuffle::internal