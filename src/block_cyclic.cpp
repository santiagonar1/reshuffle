#include "block_cyclic.hpp"

namespace reshuffle::internal {
    auto create_blocks(const int num_values, const int block_size, const int num_processors)
            -> std::vector<Block> {
        PROFILE_SCOPE_NAMED("create_blocks_1d");

        if (num_values == 0) { return {}; }

        auto blocks = std::vector<Block>{};

        for (int i = 0; i < num_values; i += block_size) {
            const auto starting_index = i;
            const auto last_index = starting_index + block_size;
            const auto owner = static_cast<RankId>(blocks.size()) % num_processors;
            blocks.emplace_back(Interval{starting_index, last_index}, owner);
        }


        const Block last_block{Interval{blocks.back().get_interval().get_left_bound(), num_values},
                               blocks.back().get_owner()};
        blocks.pop_back();
        blocks.push_back(last_block);


        return blocks;
    }
}// namespace reshuffle::internal
