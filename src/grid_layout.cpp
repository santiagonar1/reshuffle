#include "grid_layout.hpp"

#include <chrono>
#include <ranges>

namespace reshuffle::dev {
    GridLayout::GridLayout(std::vector<Block> blocks) : _blocks(std::move(blocks)) {}

    auto GridLayout::get_blocks() const -> const std::vector<Block> & { return _blocks; }

    auto GridLayout::get_block_owner(int block_id) const -> rank_id {
        if (block_id < 0 || block_id >= _blocks.size()) {
            throw std::out_of_range("block_id out of range");
        }

        return _blocks[block_id].get_owner();
    }

    auto GridLayout::get_overlay(const GridLayout &target_grid) const -> GridOverlay {
        if (target_grid.get_blocks().front().get_interval().get_left_bound() !=
            _blocks.front().get_interval().get_left_bound()) {
            throw std::invalid_argument("target_grid does not start at same index as this grid");
        }

        if (target_grid.get_blocks().back().get_interval().get_right_bound() !=
            _blocks.back().get_interval().get_right_bound()) {
            throw std::invalid_argument("target_grid does not end at same index as this grid");
        }

        auto sub_blocks = std::vector<Block>{};
        auto owners_target_grid = std::vector<rank_id>{};

        int pos_target_grid{};
        for (const auto &block: _blocks) {
            while (pos_target_grid < target_grid._blocks.size()) {
                const auto block_overlay = block.get_overlay(target_grid._blocks[pos_target_grid]);
                if (not block_overlay.has_value()) { break; }
                sub_blocks.emplace_back(block_overlay.value());
                owners_target_grid.emplace_back(target_grid.get_block_owner(pos_target_grid));
                ++pos_target_grid;
            }

            const auto last_overlay = sub_blocks.back();
            const auto checked_all_target_grid_blocks =
                    pos_target_grid == target_grid._blocks.size();
            const auto there_are_missing_blocks = last_overlay.get_interval().get_right_bound() !=
                                                  _blocks.back().get_interval().get_right_bound();
            const auto have_not_checked_all_blocks_but_missed_some =
                    not checked_all_target_grid_blocks and
                    last_overlay.get_interval().get_right_bound() <
                            target_grid._blocks[pos_target_grid].get_interval().get_left_bound();

            if ((checked_all_target_grid_blocks and there_are_missing_blocks) or
                have_not_checked_all_blocks_but_missed_some) {
                --pos_target_grid;
            }
        }

        return GridOverlay{GridLayout{std::move(sub_blocks)}, std::move(owners_target_grid)};
    }

    auto GridLayout::get_local_grid(const rank_id rank) const -> GridLayout {

        auto local_blocks_view = _blocks | std::views::filter([rank](const auto &block) {
                                     return block.get_owner() == rank;
                                 });

        if (local_blocks_view.empty()) { return GridLayout{std::vector<Block>{}}; }

        const auto first_block = local_blocks_view.front();
        const auto num_elements_first_block = first_block.get_interval().get_length();

        auto local_blocks = std::vector<Block>{};
        local_blocks.emplace_back(Block{{0, num_elements_first_block}, rank});

        for (const auto &block: local_blocks_view | std::views::drop(1)) {
            const auto last_block = local_blocks.back();
            const auto num_elements_current_block = block.get_interval().get_length();
            local_blocks.emplace_back(Block{
                    {last_block.get_interval().get_right_bound(),
                     last_block.get_interval().get_right_bound() + num_elements_current_block},
                    rank});
        }

        return GridLayout{std::move(local_blocks)};
    }
}// namespace reshuffle::dev