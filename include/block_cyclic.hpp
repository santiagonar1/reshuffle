#ifndef RESHUFFLE_BLOCK_CYCLIC_HPP
#define RESHUFFLE_BLOCK_CYCLIC_HPP

#include "block.hpp"
#include "rank_id.hpp"
#include <vector>

namespace reshuffle {
    class BlockCyclic {
    public:
        explicit BlockCyclic(int block_size, int num_values, int num_ranks);

        [[nodiscard]] auto get_blocks() const -> std::vector<Block>;
        [[nodiscard]] auto get_num_values() const -> int;
        [[nodiscard]] auto get_num_values(rank_id rank_id) const -> int;
        [[nodiscard]] auto get_rank_id(std::size_t index) const -> rank_id;
        [[nodiscard]] auto get_num_ranks() const -> int;

    private:
        const int _num_ranks;
        const int _num_values;
        const std::vector<Block> _blocks;
        const int _block_size;
    };

    auto make_block_wise(int num_values, int num_blocks) -> BlockCyclic;
}// namespace reshuffle


#endif//RESHUFFLE_BLOCK_CYCLIC_HPP
