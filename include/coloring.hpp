#ifndef RESHUFFLE_COLORING_HPP
#define RESHUFFLE_COLORING_HPP

#include "data_distribution.hpp"
#include "dimensions.hpp"
#include "indices.hpp"
#include "left_closed_range.hpp"
#include "rank_id.hpp"
#include "subdomain.hpp"
#include "utils.hpp"
#include <algorithm>
#include <functional>
#include <numeric>

namespace reshuffle {
    namespace internal {
        rank_id get_color(const std::vector<Subdomain> &subdomains, int i) {
            auto it = std::ranges::find_if(
                    subdomains, [i](const auto &subdomain) { return subdomain.contains(i); });
            //TODO: Should we check whether the index requested is out of bounds?
            return static_cast<int>(std::distance(subdomains.begin(), it));
        }

        Indices2D to_2D(int num_columns, int index) {
            return {index % num_columns, index / num_columns};
        }

        auto get_subdomains_2D(const Dimensions2D &global_dimensions,
                               const std::array<BlockWise, 2> &data_distributions) {
            const auto subdomains_x =
                    data_distributions[0].get_subdomains(global_dimensions.num_columns);
            const auto subdomains_y =
                    data_distributions[1].get_subdomains(global_dimensions.num_rows);
            return internal::combine(subdomains_x, subdomains_y);
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
        const auto subdomains = data_distribution.get_subdomains(num_values);

        auto new_global_coloring = std::vector<rank_id>(global_coloring.size());
        auto local_coloring = std::vector<rank_id>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            new_global_coloring[i] = internal::get_color(subdomains, i);
            if (rank == global_coloring[i]) { local_coloring.push_back(new_global_coloring[i]); }
        }

        return ColoringReturn{new_global_coloring, local_coloring};
    }

    auto create_coloring(const std::vector<rank_id> &global_coloring,
                         const Dimensions2D &global_dimensions,
                         const std::array<BlockWise, 2> &data_distributions, rank_id rank) {
        const auto subdomains = internal::get_subdomains_2D(global_dimensions, data_distributions);

        auto new_global_coloring = std::vector<rank_id>(global_coloring.size());
        auto local_coloring = std::vector<rank_id>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            const auto [x_coord, y_coord] = internal::to_2D(global_dimensions.num_columns, i);

            auto it = std::ranges::find_if(subdomains, [x_coord, y_coord](const auto &r) {
                return r.first.contains(x_coord) and r.second.contains(y_coord);
            });
            new_global_coloring[i] = static_cast<int>(std::distance(subdomains.begin(), it));
            if (rank == global_coloring[i]) { local_coloring.push_back(new_global_coloring[i]); }
        }

        return ColoringReturn{new_global_coloring, local_coloring};
    }

    auto get_subdomain_dimension(const BlockWise &data_distribution, int num_values, rank_id rank) {
        const auto coloring_descriptor = data_distribution.get_subdomains(num_values);

        if (rank >= coloring_descriptor.size()) { return 0; }

        return coloring_descriptor[rank].get_right_bound() -
               coloring_descriptor[rank].get_left_bound();
    }

    auto get_subdomain_dimension(const std::array<BlockWise, 2> &data_distributions,
                                 Dimensions2D global_dimensions, rank_id rank) {
        const auto subdomains = internal::get_subdomains_2D(global_dimensions, data_distributions);

        if (rank >= subdomains.size()) { return Dimensions2D{0, 0}; }

        const auto &subdomain = subdomains[rank];
        const auto num_rows =
                subdomain.second.get_right_bound() - subdomain.second.get_left_bound();
        const auto num_columns =
                subdomain.first.get_right_bound() - subdomain.first.get_left_bound();

        return Dimensions2D{num_rows, num_columns};
    }
}// namespace reshuffle

#endif//RESHUFFLE_COLORING_HPP
