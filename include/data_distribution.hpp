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
}// namespace reshuffle


#endif//RESHUFFLE_DATA_DISTRIBUTION_HPP
