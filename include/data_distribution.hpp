#ifndef RESHUFFLE_DATA_DISTRIBUTION_HPP
#define RESHUFFLE_DATA_DISTRIBUTION_HPP

#include "block.hpp"
#include <cmath>
#include <vector>

namespace reshuffle {
    class BlockCyclic {
    private:
        const int _block_size;
        const int _num_values;

    public:
        explicit BlockCyclic(int block_size, int num_values)
            : _block_size(block_size), _num_values(num_values) {}

        [[nodiscard]] auto get_blocks() const {
            std::vector<Block> blocks{};

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
    };

    auto make_block_wise(int num_values, int num_blocks) -> BlockCyclic;

    class BlockWise {
    private:
        const int _num_blocks;

        [[nodiscard]] int get_min_values_per_block(int num_values) const {
            return num_values / _num_blocks;
        }

    public:
        explicit BlockWise(int num_blocks) : _num_blocks(num_blocks) {}

        [[nodiscard]] auto get_blocks(int num_values) const {
            const auto min_values_per_block = get_min_values_per_block(num_values);
            std::vector<Block> blocks{};

            for (int i = 0; i < min_values_per_block * _num_blocks; i += min_values_per_block) {
                const auto starting_index = i;
                const auto last_index = starting_index + min_values_per_block;
                blocks.emplace_back(starting_index, last_index);
            }

            Block last_block{blocks.back().get_left_bound(), num_values};
            blocks.pop_back();
            blocks.push_back(last_block);
            return blocks;
        }
    };
}// namespace reshuffle


#endif//RESHUFFLE_DATA_DISTRIBUTION_HPP
