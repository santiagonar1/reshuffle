#include "multidimensional_block.hpp"

#include "utils.hpp"

namespace reshuffle::internal {
    auto get_dimensions(const std::vector<Block> &blocks) -> int {
        const auto ordered_blocks = sort(blocks);
        const auto smallest_coordinate = ordered_blocks.front().get_interval().get_left_bound();
        const auto largest_coordinate = ordered_blocks.back().get_interval().get_right_bound();
        return largest_coordinate - smallest_coordinate;
    }

}// namespace reshuffle::internal