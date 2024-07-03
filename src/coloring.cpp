#include "coloring.hpp"

#include "utils.hpp"
#include <algorithm>
#include <functional>

namespace reshuffle {
    namespace internal {
        auto to_2D(int num_columns, int index) -> Indices2D {
            return {index % num_columns, index / num_columns};
        }

        auto get_blocks_2D(const std::array<BlockCyclic, 2> &data_distributions)
                -> std::vector<std::pair<LeftClosedRange, LeftClosedRange>> {
            const auto blocks_x = data_distributions[0].get_blocks();
            const auto blocks_y = data_distributions[1].get_blocks();
            return internal::combine(blocks_x, blocks_y);
        }

        auto throw_if_different(int val1, int val2, const std::string &error_msg) {
            if (val1 != val2) { throw std::invalid_argument(error_msg); }
        }

        auto get_dimension_from_distribution(const std::array<BlockCyclic, 2> &distribution)
                -> Dimension<2> {
            return Dimension<2>{distribution[0].get_num_values(), distribution[1].get_num_values()};
        }
    }// namespace internal

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const BlockCyclic &data_distribution, rank_id rank) -> ColoringReturn {

        internal::throw_if_different(static_cast<int>(global_coloring.size()),
                                     data_distribution.get_num_values(),
                                     std::string{"Mismatch between size of global_coloring and "
                                                 "number of values data_distribution"});

        auto new_global_coloring = std::vector<rank_id>(global_coloring.size());
        auto local_coloring = std::vector<rank_id>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            new_global_coloring[i] = data_distribution.get_rank_id(i);
            if (rank == global_coloring[i]) { local_coloring.push_back(new_global_coloring[i]); }
        }

        return ColoringReturn{new_global_coloring, local_coloring};
    }

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const std::array<BlockCyclic, 2> &data_distributions,
                         rank_id rank) -> ColoringReturn {

        const auto global_dimensions = Dimension<2>{data_distributions[0].get_num_values(),
                                                    data_distributions[1].get_num_values()};

        internal::throw_if_different(static_cast<int>(global_coloring.size()),
                                     calc_total_num_values(global_dimensions),
                                     std::string{"Mismatch between size of global_coloring and "
                                                 "number of elements global_dimensions"});

        const auto blocks = internal::get_blocks_2D(data_distributions);

        auto new_global_coloring = std::vector<rank_id>(global_coloring.size());
        auto local_coloring = std::vector<rank_id>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            const auto [x_coord, y_coord] = internal::to_2D(global_dimensions[0], i);

            auto it = std::ranges::find_if(blocks, [x_coord, y_coord](const auto &r) {
                return r.first.contains(x_coord) and r.second.contains(y_coord);
            });
            new_global_coloring[i] = static_cast<int>(std::distance(blocks.begin(), it));
            if (rank == global_coloring[i]) { local_coloring.push_back(new_global_coloring[i]); }
        }

        return ColoringReturn{new_global_coloring, local_coloring};
    }

    auto get_global_coloring(const std::array<BlockCyclic, 2> &data_distributions)
            -> std::vector<rank_id> {
        const auto dimensions = internal::get_dimension_from_distribution(data_distributions);
        const auto num_values = calc_total_num_values(dimensions);
        const auto dummy_global_coloring = std::vector<rank_id>(num_values);
        const rank_id dummy_rank = 0;
        const auto [global_coloring, _] =
                create_coloring(dummy_global_coloring, data_distributions, dummy_rank);
        return global_coloring;
    }

    auto get_block_dimension(const BlockCyclic &data_distribution, rank_id rank) -> int {
        const auto blocks = data_distribution.get_blocks();

        if (rank >= blocks.size()) { return 0; }

        return blocks[rank].get_length();
    }

    auto get_block_dimension(const std::array<BlockCyclic, 2> &data_distributions,
                             rank_id rank) -> Dimension<2> {
        const auto block_pairs = internal::get_blocks_2D(data_distributions);

        if (rank >= block_pairs.size()) { return Dimension<2>{{0, 0}}; }

        const auto &block_pair = block_pairs[rank];
        const auto num_values_x = block_pair.first.get_length();
        const auto num_values_y = block_pair.second.get_length();

        return Dimension<2>{{num_values_x, num_values_y}};
    }
}// namespace reshuffle
