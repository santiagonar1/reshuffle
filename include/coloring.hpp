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
                         const Dimension<2> &global_dimensions,
                         const std::array<BlockCyclic, 2> &data_distributions,
                         rank_id rank) -> ColoringReturn;

    auto get_block_dimension(const BlockCyclic &data_distribution, rank_id rank) -> int;

    auto get_block_dimension(const std::array<BlockCyclic, 2> &data_distributions,
                             rank_id rank) -> Dimension<2>;
}// namespace reshuffle

#endif//RESHUFFLE_COLORING_HPP
