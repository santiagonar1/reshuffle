#include "communication_package.hpp"

namespace reshuffle::internal {
    auto get_starting_positions(const std::vector<Block> &blocks) -> std::map<rank_id, int> {
        PROFILE_SCOPE_NAMED("get_starting_positions");
        auto starting_positions = std::map<rank_id, int>{};
        for (const auto &block: blocks) {
            const auto owner = block.get_owner();
            const auto starting_position = block.get_interval().get_left_bound();
            starting_positions.emplace(owner, starting_position);
        }

        return starting_positions;
    }
}// namespace reshuffle::internal