#include "block.hpp"

#include <map>
#include <ranges>

namespace reshuffle::dev {

    Block::Block() : _interval{0, 0}, _owner{-1} {};

    Block::Block(internal::LeftClosedRange interval, const rank_id owner)
        : _interval{std::move(interval)}, _owner{owner} {};

    auto Block::get_interval() const -> const internal::LeftClosedRange & { return _interval; }

    auto Block::get_owner() const -> rank_id { return _owner; }

    [[nodiscard]] auto Block::get_overlay(const Block &other) const -> std::optional<Block> {
        const auto interval_overlay = _interval.get_overlay(other._interval);

        if (not interval_overlay.has_value()) { return std::nullopt; }

        return Block{interval_overlay.value(), _owner};
    }

    auto Block::operator==(const Block &other) const -> bool {
        return _interval == other._interval && _owner == other._owner;
    }

    auto Block::operator=(const Block &other) -> Block & = default;

    auto join(const std::vector<Block> &blocks) -> std::vector<Block> {
        if (blocks.empty()) { return {}; }

        const auto first_block = blocks.front();
        const auto num_elements_first_block = first_block.get_interval().get_length();

        auto joined_blocks = std::vector<Block>{};
        joined_blocks.emplace_back(Block{{0, num_elements_first_block}, first_block.get_owner()});

        for (const auto &block: blocks | std::views::drop(1)) {
            const auto last_block = joined_blocks.back();
            const auto num_elements_current_block = block.get_interval().get_length();
            joined_blocks.emplace_back(Block{
                    {last_block.get_interval().get_right_bound(),
                     last_block.get_interval().get_right_bound() + num_elements_current_block},
                    block.get_owner()});
        }

        return joined_blocks;
    }

    auto get_num_elements_per_processor(const std::vector<Block> &blocks)
            -> std::map<rank_id, int> {
        auto num_elements_per_process = std::map<rank_id, int>{};
        for (const auto &block: blocks) {
            const auto owner = block.get_owner();
            const auto num_elements = block.get_interval().get_length();
            num_elements_per_process[owner] += num_elements;
        }
        return num_elements_per_process;
    }

    auto group_by_processor(const std::vector<Block> &blocks) -> std::vector<Block> {
        if (blocks.empty()) { return {}; }

        const auto num_elements_per_process = get_num_elements_per_processor(blocks);
        auto blocks_grouped_by_owner = std::vector<Block>{};

        const auto [first_rank, first_length] = *num_elements_per_process.begin();
        blocks_grouped_by_owner.emplace_back(Block{{0, first_length}, first_rank});

        for (const auto &[rank, num_elements]: num_elements_per_process | std::views::drop(1)) {
            const auto last_inserted_block = blocks_grouped_by_owner.back();
            blocks_grouped_by_owner.emplace_back(
                    Block{{last_inserted_block.get_interval().get_right_bound(),
                           last_inserted_block.get_interval().get_right_bound() + num_elements},
                          rank});
        }

        return blocks_grouped_by_owner;
    }
}// namespace reshuffle::dev