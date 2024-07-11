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


namespace reshuffle::internal {
    struct ColoringReturn {
        std::vector<rank_id> global_coloring;
        std::vector<rank_id> local_coloring;
    };

    auto get_global_and_local_coloring(const std::vector<rank_id> &global_coloring,
                                       const BlockCyclic &data_distribution,
                                       rank_id rank) -> ColoringReturn;

    auto get_global_and_local_coloring(const std::vector<rank_id> &global_coloring,
                                       const std::array<BlockCyclic, 2> &data_distributions,
                                       rank_id rank) -> ColoringReturn;

    auto get_global_coloring(const BlockCyclic &data_distribution) -> std::vector<rank_id>;

    auto get_global_coloring(const std::array<BlockCyclic, 2> &data_distributions)
            -> std::vector<rank_id>;

    auto get_block_dimension(const BlockCyclic &data_distribution, rank_id rank) -> int;

    auto get_block_dimension(const std::array<BlockCyclic, 2> &data_distributions,
                             rank_id rank) -> Dimension<2>;
}// namespace reshuffle::internal

#endif//RESHUFFLE_COLORING_HPP
