#ifndef RESHUFFLE_COLORING_HPP
#define RESHUFFLE_COLORING_HPP

#include "block.hpp"
#include "data_distribution.hpp"
#include "dimensions.hpp"
#include "indices.hpp"
#include "left_closed_range.hpp"
#include "rank_id.hpp"
#include "utils.hpp"
#include <algorithm>
#include <functional>
#include <numeric>

namespace reshuffle {
    namespace internal {
        rank_id get_color(const std::vector<Block> &blocks, int i) {
            auto it = std::ranges::find_if(blocks,
                                           [i](const auto &block) { return block.contains(i); });
            //TODO: Should we check whether the index requested is out of bounds?
            return static_cast<int>(std::distance(blocks.begin(), it));
        }

        Indices2D to_2D(int num_columns, int index) {
            return {index % num_columns, index / num_columns};
        }

        auto get_blocks_2D(const Dimensions2D &global_dimensions,
                           const std::array<BlockWise, 2> &data_distributions) {
            const auto blocks_x = data_distributions[0].get_blocks(global_dimensions.num_columns);
            const auto blocks_y = data_distributions[1].get_blocks(global_dimensions.num_rows);
            return internal::combine(blocks_x, blocks_y);
        }

    }// namespace internal

    struct ColoringReturn {
        std::vector<rank_id> global_coloring;
        std::vector<rank_id> local_coloring;

        [[nodiscard]] auto as_tuple() const {
            return std::make_tuple(global_coloring, local_coloring);
        }
    };

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const BlockWise &data_distribution, rank_id rank) {
        const auto num_values = static_cast<int>(global_coloring.size());
        const auto blocks = data_distribution.get_blocks(num_values);

        auto new_global_coloring = std::vector<rank_id>(global_coloring.size());
        auto local_coloring = std::vector<rank_id>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            new_global_coloring[i] = internal::get_color(blocks, i);
            if (rank == global_coloring[i]) { local_coloring.push_back(new_global_coloring[i]); }
        }

        return ColoringReturn{new_global_coloring, local_coloring};
    }

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const Dimensions2D &global_dimensions,
                         const std::array<BlockWise, 2> &data_distributions, rank_id rank) {
        const auto blocks = internal::get_blocks_2D(global_dimensions, data_distributions);

        auto new_global_coloring = std::vector<rank_id>(global_coloring.size());
        auto local_coloring = std::vector<rank_id>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            const auto [x_coord, y_coord] = internal::to_2D(global_dimensions.num_columns, i);

            auto it = std::ranges::find_if(blocks, [x_coord, y_coord](const auto &r) {
                return r.first.contains(x_coord) and r.second.contains(y_coord);
            });
            new_global_coloring[i] = static_cast<int>(std::distance(blocks.begin(), it));
            if (rank == global_coloring[i]) { local_coloring.push_back(new_global_coloring[i]); }
        }

        return ColoringReturn{new_global_coloring, local_coloring};
    }

    auto get_block_dimension(const BlockWise &data_distribution, int num_values, rank_id rank) {
        const auto blocks = data_distribution.get_blocks(num_values);

        if (rank >= blocks.size()) { return 0; }

        return blocks[rank].get_length();
    }

    auto get_block_dimension(const std::array<BlockWise, 2> &data_distributions,
                             Dimensions2D global_dimensions, rank_id rank) {
        const auto block_pairs = internal::get_blocks_2D(global_dimensions, data_distributions);

        if (rank >= block_pairs.size()) { return Dimensions2D{0, 0}; }

        const auto &block_pair = block_pairs[rank];
        const auto num_rows = block_pair.second.get_length();
        const auto num_columns = block_pair.first.get_length();

        return Dimensions2D{num_rows, num_columns};
    }
}// namespace reshuffle

#endif//RESHUFFLE_COLORING_HPP
