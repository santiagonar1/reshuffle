#ifndef RESHUFFLE_COLORING_HPP
#define RESHUFFLE_COLORING_HPP

#include <functional>
#include <numeric>
#include <algorithm>
#include "utils.hpp"
#include "dimensions.hpp"
#include "left_closed_range.hpp"
#include "rank_id.hpp"

namespace reshuffle {
    using ColoringDescriptor = std::vector<internal::LeftClosedRange>;

    class BlockWise {
    private:
        const int _num_blocks;

        [[nodiscard]] int get_min_values_per_block(int num_values) const {
            return num_values / _num_blocks;
        }

    public:
        explicit BlockWise(int num_blocks) : _num_blocks(num_blocks) {}

        [[nodiscard]] ColoringDescriptor get_coloring_descriptor(int num_values) const {
            const auto min_values_per_block = get_min_values_per_block(num_values);
            ColoringDescriptor coloring_descriptor{};

            for (int i = 0; i < min_values_per_block * _num_blocks; i += min_values_per_block) {
                const auto starting_index = i;
                const auto last_index = starting_index + min_values_per_block;
                coloring_descriptor.emplace_back(starting_index, last_index);
            }

            coloring_descriptor.back().second = num_values;
            return coloring_descriptor;
        }
    };

    namespace internal {
        rank_id get_color(const ColoringDescriptor &coloring_descriptor, int i) {
            auto it = std::ranges::find_if(coloring_descriptor, [i](const auto &r) { return in_range(r, i); });
            //TODO: Should we check whether the index requested is out of bounds?
            return static_cast<int>(std::distance(coloring_descriptor.begin(), it));
        }

        std::pair<int, int> to_2D(int num_columns, int index) {
            return {index % num_columns, index / num_columns};
        }

        auto get_subdomains(const Dimensions2D &global_dimensions,
                            const std::array<BlockWise, 2> &strategies) {
            const auto coloring_x = strategies[0].get_coloring_descriptor(global_dimensions.num_columns);
            const auto coloring_y = strategies[1].get_coloring_descriptor(global_dimensions.num_rows);
            return internal::combine(coloring_x, coloring_y);
        }

    }

    struct ColoringReturn {
        std::vector<rank_id> global_coloring;
        std::vector<rank_id> local_coloring;
    };

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const BlockWise &strategy, rank_id rank) {
        const auto num_values = static_cast<int>(global_coloring.size());
        const auto coloring_descriptor = strategy.get_coloring_descriptor(num_values);

        auto new_global_coloring = std::vector<rank_id>(global_coloring.size());
        auto local_coloring = std::vector<rank_id>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            new_global_coloring[i] = internal::get_color(coloring_descriptor, i);
            if (rank == global_coloring[i]) {
                local_coloring.push_back(new_global_coloring[i]);
            }
        }

        return ColoringReturn{new_global_coloring, local_coloring};
    }

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const Dimensions2D &global_dimensions,
                         const std::array<BlockWise, 2> &strategies, rank_id rank) {
        const auto combination = internal::get_subdomains(global_dimensions, strategies);

        auto new_global_coloring = std::vector<rank_id>(global_coloring.size());
        auto local_coloring = std::vector<rank_id>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            const auto [x_coord, y_coord] = internal::to_2D(global_dimensions.num_columns, i);

            auto it = std::ranges::find_if(combination, [x_coord, y_coord](const auto &r) {
                return internal::in_range(r.first, x_coord) and internal::in_range(r.second, y_coord);
            });
            new_global_coloring[i] = static_cast<int>(std::distance(combination.begin(), it));
            if (rank == global_coloring[i]) {
                local_coloring.push_back(new_global_coloring[i]);
            }
        }

        return ColoringReturn{new_global_coloring, local_coloring};
    }

    auto get_subdomain_dimension(const BlockWise &strategy, int num_values, rank_id rank) {
        const auto coloring_descriptor = strategy.get_coloring_descriptor(num_values);

        if (rank >= coloring_descriptor.size()) {
            return 0;
        }

        return coloring_descriptor[rank].second - coloring_descriptor[rank].first;
    }

    auto get_subdomain_dimension(const std::array<BlockWise, 2> &strategies, Dimensions2D global_dimensions,
                                 rank_id rank) {
        const auto combination = internal::get_subdomains(global_dimensions, strategies);

        if (rank >= combination.size()) {
            return Dimensions2D{0, 0};
        }

        const auto &subdomain_range = combination[rank];
        const auto num_rows = subdomain_range.second.second - subdomain_range.second.first;
        const auto num_columns = subdomain_range.first.second - subdomain_range.first.first;

        return Dimensions2D{num_rows, num_columns};
    }
}

#endif //RESHUFFLE_COLORING_HPP
