#include "block_cyclic.hpp"

#include <cmath>
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

    BlockCyclic::BlockCyclic(const int block_size, const int total_num_values, const int num_ranks)
        : _num_ranks(num_ranks), _total_num_values(total_num_values),
          _blocks(internal::get_blocks(block_size, total_num_values)), _block_size(block_size) {}

    auto BlockCyclic::get_blocks() const -> std::vector<Block> { return _blocks; }

    auto BlockCyclic::get_rank_id(std::size_t index) const -> rank_id {
        const auto global_block_id = static_cast<int>(index) / _block_size;
        return global_block_id % _num_ranks;
    }

    auto BlockCyclic::get_num_ranks() const -> int { return _num_ranks; }

    auto BlockCyclic::get_num_total_values() const -> int { return _total_num_values; }

    auto BlockCyclic::get_num_values_hold_by(const rank_id rank_id) const -> int {
        if (rank_id >= _num_ranks) { return 0; }

        if (rank_id < 0) { throw std::invalid_argument("rank_id cannot be negative"); }

        const auto num_blocks = static_cast<int>(_blocks.size());
        const auto min_blocks_per_rank = num_blocks / _num_ranks;
        const auto max_id_with_extra_blocks = (num_blocks % _num_ranks) - 1;

        const auto min_values_per_rank = _block_size * min_blocks_per_rank;

        if (rank_id == max_id_with_extra_blocks) {
            const auto remaining_values = _total_num_values % _block_size;
            const auto num_values_last_block =
                    remaining_values == 0 ? _block_size : remaining_values;
            return min_values_per_rank + num_values_last_block;
        }

        if (rank_id < max_id_with_extra_blocks) { return min_values_per_rank + _block_size; }

        return min_values_per_rank;
    }

    auto BlockCyclic::operator==(const BlockCyclic &other) const -> bool {
        return _num_ranks == other._num_ranks and _total_num_values == other._total_num_values and
               _block_size == other._block_size;
    }

    auto make_block_wise(const int num_values, const int num_blocks) -> BlockCyclic {
        if (num_blocks == 0) { throw std::invalid_argument("num_blocks cannot be zero"); }
        const int block_size = std::ceil(static_cast<double>(num_values) / num_blocks);
        return BlockCyclic(block_size, num_values, num_blocks);
    }

}// namespace reshuffle