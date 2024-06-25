#ifndef RESHUFFLE_DATA_DISTRIBUTION_HPP
#define RESHUFFLE_DATA_DISTRIBUTION_HPP

#include "block.hpp"
#include "rank_id.hpp"
#include <vector>

namespace reshuffle {
    class BlockCyclic {
    private:
        const int _num_procs;
        const int _num_values;
        const std::vector<Block> _blocks;

    public:
        explicit BlockCyclic(int block_size, int num_values, int num_procs);

        [[nodiscard]] auto get_blocks() const -> std::vector<Block>;
        [[nodiscard]] auto get_num_values() const -> int;
        [[nodiscard]] auto get_rank_id(std::size_t num_procs, std::size_t index) const -> rank_id;
    };

    auto make_block_wise(int num_values, int num_blocks) -> BlockCyclic;
}// namespace reshuffle


#endif//RESHUFFLE_DATA_DISTRIBUTION_HPP
