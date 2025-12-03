#include "block_overlay.hpp"

#include <ostream>

namespace reshuffle::internal {

    auto check_bounds(const std::vector<Block> &origin_blocks,
                      const std::vector<Block> &target_blocks) -> void;

    auto get_blocks_overlay(const Block &origin, const Block &target)
            -> std::optional<BlockOverlay> {
        if (const auto interval_overlay = origin.get_interval().get_overlay(target.get_interval());
            interval_overlay.has_value()) {
            return BlockOverlay{.interval = interval_overlay.value(),
                                .id_origin = origin.get_owner(),
                                .id_target = target.get_owner()};
        }
        return std::nullopt;
    }

    auto get_blocks_overlay(const std::vector<Block> &origin_blocks,
                            const std::vector<Block> &target_blocks) -> std::vector<BlockOverlay> {
        check_bounds(origin_blocks, target_blocks);

        auto result = std::vector<BlockOverlay>{};

        int pos_target_grid{};
        for (const auto &origin_block: origin_blocks) {
            auto overlay = get_blocks_overlay(origin_block, target_blocks[pos_target_grid]);
            while (overlay.has_value()) {
                result.push_back(overlay.value());
                pos_target_grid++;
                if (pos_target_grid == target_blocks.size()) {
                    pos_target_grid--;
                    break;
                }
                overlay = get_blocks_overlay(origin_block, target_blocks[pos_target_grid]);
            }

            const auto current_target_interval = target_blocks[pos_target_grid].get_interval();
            const auto current_overlay_interval = result.back().interval;

            const auto missed_an_overlap = current_overlay_interval.get_right_bound() <
                                           current_target_interval.get_left_bound();

            if (missed_an_overlap) { pos_target_grid--; }
        }

        return result;
    }

    auto operator<<(std::ostream &os, const BlockOverlay &block_overlay) -> std::ostream & {
        return os << "[Interval: " << block_overlay.interval
                  << ", id_origin: " << block_overlay.id_origin
                  << ", id_target: " << block_overlay.id_target << "]";
    }

    auto check_bounds(const std::vector<Block> &origin_blocks,
                      const std::vector<Block> &target_blocks) -> void {
        if (origin_blocks.front().get_interval().get_left_bound() !=
            target_blocks.front().get_interval().get_left_bound()) {
            const auto error_msg = std::format(
                    "origin_blocks does not start at same index as target_blocks: {} vs {}",
                    origin_blocks.front().get_interval().get_left_bound(),
                    target_blocks.front().get_interval().get_left_bound());
            throw std::invalid_argument(error_msg);
        }

        if (origin_blocks.back().get_interval().get_right_bound() !=
            target_blocks.back().get_interval().get_right_bound()) {
            const auto error_msg = std::format(
                    "origin_blocks does not end at same index as target_blocks: {} vs {}",
                    origin_blocks.back().get_interval().get_right_bound(),
                    target_blocks.back().get_interval().get_right_bound());
            throw std::invalid_argument(error_msg);
        }
    }
}// namespace reshuffle::internal