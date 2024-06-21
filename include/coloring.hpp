#ifndef RESHUFFLE_COLORING_HPP
#define RESHUFFLE_COLORING_HPP

#include "block.hpp"
#include "data_distribution.hpp"
#include "dimensions.hpp"
#include "indices.hpp"
#include "left_closed_range.hpp"
#include "rank_id.hpp"
#include <array>
#include <functional>
#include <vector>


namespace reshuffle {
    namespace internal {
        auto get_color(const std::vector<Block> &blocks, int i) -> rank_id;

        auto to_2D(int num_columns, int index) -> Indices2D;

        auto get_blocks_2D(const std::array<BlockCyclic, 2> &data_distributions)
                -> std::vector<std::pair<LeftClosedRange, LeftClosedRange>>;

    }// namespace internal

    struct ColoringReturn {
        std::vector<rank_id> global_coloring;
        std::vector<rank_id> local_coloring;

        [[nodiscard]] auto as_tuple() const {
            return std::make_tuple(global_coloring, local_coloring);
        }
    };

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const BlockCyclic &data_distribution, rank_id rank) -> ColoringReturn;

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const Dimensions2D &global_dimensions,
                         const std::array<BlockCyclic, 2> &data_distributions,
                         rank_id rank) -> ColoringReturn;

    auto get_block_dimension(const BlockCyclic &data_distribution, rank_id rank) -> int;

    auto get_block_dimension(const std::array<BlockCyclic, 2> &data_distributions,
                             rank_id rank) -> Dimensions2D;
}// namespace reshuffle

#endif//RESHUFFLE_COLORING_HPP
