#include "data_distribution.hpp"

#include <cmath>

namespace reshuffle {
    namespace internal {
        auto get_blocks(int block_size, int num_values) -> std::vector<Block> {
            std::vector<Block> blocks{};

            for (int i = 0; i < num_values; i += block_size) {
                const auto starting_index = i;
                const auto last_index = starting_index + block_size;
                blocks.emplace_back(starting_index, last_index);
            }

            Block last_block{blocks.back().get_left_bound(), num_values};
            blocks.pop_back();
            blocks.push_back(last_block);
            return blocks;
        }
    }// namespace internal

    BlockCyclic::BlockCyclic(int block_size, int num_values)
        : _block_size(block_size), _num_values(num_values),
          _blocks(internal::get_blocks(block_size, num_values)) {}

    auto BlockCyclic::get_blocks() const -> std::vector<Block> { return _blocks; }

        for (int i = 0; i < _num_values; i += _block_size) {
            const auto starting_index = i;
            const auto last_index = starting_index + _block_size;
            blocks.emplace_back(starting_index, last_index);
        }

        Block last_block{blocks.back().get_left_bound(), _num_values};
        blocks.pop_back();
        blocks.push_back(last_block);
        return blocks;
    }

    auto BlockCyclic::get_num_values() const -> int { return _num_values; }

    auto make_block_wise(int num_values, int num_blocks) -> BlockCyclic {
        const int block_size = std::ceil(static_cast<double>(num_values) / num_blocks);
        return BlockCyclic(block_size, num_values);
    }

}// namespace reshuffle