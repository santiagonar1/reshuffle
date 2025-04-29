#ifndef RESHUFFLE_BLOCK_CYCLIC_HPP
#define RESHUFFLE_BLOCK_CYCLIC_HPP

#include "block.hpp"
#include "grid_layout.hpp"
#include "processor_grid.hpp"
#include "rank_id.hpp"

#include <vector>

namespace reshuffle {
    class BlockCyclic {
    public:
        explicit BlockCyclic(int block_size, int total_num_values, int num_ranks);

        [[nodiscard]] auto get_blocks() const -> std::vector<Block>;
        [[nodiscard]] auto get_num_total_values() const -> int;
        [[nodiscard]] auto get_num_values_hold_by(rank_id rank_id) const -> int;
        [[nodiscard]] auto get_rank_id(std::size_t index) const -> rank_id;
        [[nodiscard]] auto get_num_ranks() const -> int;

        auto operator==(const BlockCyclic &other) const -> bool;

    private:
        const int _num_ranks;
        const int _total_num_values;
        const std::vector<Block> _blocks;
        const int _block_size;
    };

    auto make_block_wise(int num_values, int num_blocks) -> BlockCyclic;

    namespace dev {
        class BlockCyclic {
        public:
            BlockCyclic(int num_global_values, int block_size,
                        const ProcessorGrid<1> &processor_grid);

            [[nodiscard]] auto get_grid_layout() const -> const GridLayout<1> &;
            [[nodiscard]] auto get_num_global_values() const -> int;
            [[nodiscard]] auto get_processor_grid() const -> const ProcessorGrid<1> &;

            auto operator==(const BlockCyclic &other) const -> bool;

        private:
            const int _num_global_values;
            const int _block_size;
            const ProcessorGrid<1> _processor_grid;
            const GridLayout<1> _grid_layout;
        };

        auto make_block_wise_distribution(int num_global_values,
                                          const ProcessorGrid<1> &processor_grid) -> BlockCyclic;
    }// namespace dev
}// namespace reshuffle


#endif//RESHUFFLE_BLOCK_CYCLIC_HPP
