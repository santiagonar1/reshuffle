#ifndef RESHUFFLE_DATA_DISTRIBUTION_HPP
#define RESHUFFLE_DATA_DISTRIBUTION_HPP

#include "block.hpp"
#include <vector>

namespace reshuffle {
    class BlockCyclic {
    private:
        const int _block_size;
        const int _num_values;

    public:
        explicit BlockCyclic(int block_size, int num_values);

        [[nodiscard]] auto get_blocks() const -> std::vector<Block>;
    };

    auto make_block_wise(int num_values, int num_blocks) -> BlockCyclic;
}// namespace reshuffle


#endif//RESHUFFLE_DATA_DISTRIBUTION_HPP
