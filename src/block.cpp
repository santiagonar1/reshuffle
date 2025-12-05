#include "block.hpp"

#include <compare>
#include <map>
#include <ostream>
#include <ranges>

namespace reshuffle::internal {

    Block::Block() : _interval{0, 0}, _owner{-1} {};

    Block::Block(const LeftClosedRange &interval, const RankId owner)
        : _interval{interval}, _owner{owner} {};

    auto Block::get_interval() const -> const LeftClosedRange & { return _interval; }

    auto Block::get_owner() const -> RankId { return _owner; }

    auto Block::get_num_elements() const -> int { return _interval.get_length(); }


    auto Block::operator==(const Block &other) const -> bool {
        return _interval == other._interval && _owner == other._owner;
    }

    auto Block::operator=(const Block &other) -> Block & = default;

    auto Block::operator<=>(const Block &other) const -> std::strong_ordering {
        return _interval <=> other._interval;
    }

    auto operator<<(std::ostream &os, const Block &block) -> std::ostream & {
        return os << "[Interval: " << block.get_interval() << ", id: " << block.get_owner() << "]";
    }


    auto make_contiguous(const std::vector<Block> &blocks) -> std::vector<Block> {
        if (blocks.empty()) { return {}; }

        const auto first_block = blocks.front();
        const auto num_elements_first_block = first_block.get_num_elements();

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

    auto get_num_elements_per_processor(const std::vector<Block> &blocks) -> std::map<RankId, int> {
        auto num_elements_per_process = std::map<RankId, int>{};
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
}// namespace reshuffle::internal