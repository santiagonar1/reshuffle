#include "block_cyclic.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <ranges>
#include <stdexcept>

namespace reshuffle {
    namespace internal {
        auto get_blocks(const int block_size, const int num_values) -> std::vector<Block> {
            std::vector<Block> blocks{};

            for (int i = 0; i < num_values; i += block_size) {
                const auto starting_index = i;
                const auto last_index = starting_index + block_size;
                blocks.emplace_back(starting_index, last_index);
            }

            if (not blocks.empty()) {
                const Block last_block{blocks.back().get_left_bound(), num_values};
                blocks.pop_back();
                blocks.push_back(last_block);
            }

            return blocks;
        }

        auto generate_integers(int start, int end_before, int step) -> std::vector<int> {
            auto range =
                    std::views::iota(0) |
                    std::views::transform([start, step](int i) { return start + i * step; }) |
                    std::views::take_while([end_before](int value) { return value < end_before; });

            std::vector<int> result{};
            std::ranges::copy(range, std::back_inserter(result));

            return result;
        }
    }// namespace internal

    BlockCyclic::BlockCyclic(const int block_size, const int num_values, const int num_procs)
        : _num_procs(num_procs), _num_values(num_values),
          _blocks(internal::get_blocks(block_size, num_values)) {}

    // TODO: Do we really need to expose this to our users?
    auto BlockCyclic::get_blocks() const -> std::vector<Block> { return _blocks; }

    auto BlockCyclic::get_rank_id(std::size_t index) const -> rank_id {
        const auto it = std::ranges::find_if(
                _blocks, [index](const auto &block) { return block.contains(index); });
        const auto block_id = static_cast<std::size_t>(std::distance(_blocks.begin(), it));
        return static_cast<rank_id>(block_id % _num_procs);
    }

    auto BlockCyclic::get_num_ranks() const -> int { return _num_procs; }

    auto BlockCyclic::get_num_values() const -> int { return _num_values; }

    auto BlockCyclic::get_num_values(const int rank_id) const -> int {
        const auto block_ids =
                internal::generate_integers(rank_id, static_cast<int>(_blocks.size()), _num_procs);

        const int num_values = std::accumulate(block_ids.begin(), block_ids.end(), 0,
                                               [this](const int value, const int block_id) {
                                                   return value + _blocks[block_id].get_length();
                                               });

        return num_values;
    }

    auto make_block_wise(const int num_values, const int num_blocks) -> BlockCyclic {
        if (num_blocks == 0) { throw std::invalid_argument("num_blocks cannot be zero"); }
        const int block_size = std::ceil(static_cast<double>(num_values) / num_blocks);
        return BlockCyclic(block_size, num_values, num_blocks);
    }

}// namespace reshuffle