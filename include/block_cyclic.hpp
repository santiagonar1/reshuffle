#ifndef RESHUFFLE_BLOCK_CYCLIC_HPP
#define RESHUFFLE_BLOCK_CYCLIC_HPP

#include "block.hpp"
#include "rank_id.hpp"
#include <vector>

namespace reshuffle {
    class BlockCyclic {
    public:
        explicit BlockCyclic(int block_size, int num_values, int num_procs);

        [[nodiscard]] auto get_blocks() const -> std::vector<Block>;
        [[nodiscard]] auto get_num_values() const -> int;
        [[nodiscard]] auto get_rank_id(std::size_t index) const -> rank_id;

    private:
        const int _num_procs;
        const int _num_values;
        const std::vector<Block> _blocks;
    };

    auto make_block_wise(int num_values, int num_blocks) -> BlockCyclic;
}// namespace reshuffle


#endif//RESHUFFLE_BLOCK_CYCLIC_HPP
