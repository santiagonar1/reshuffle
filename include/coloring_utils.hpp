#ifndef COLORING_UTILS_HPP
#define COLORING_UTILS_HPP

#include <vector>

#include "rank_id.hpp"

namespace reshuffle::internal {
    inline auto calc_num_values_per_rank(const int num_ranks,
                                         const std::vector<rank_id> &coloring) {
        std::vector<int> values_per_rank(num_ranks);

        for (const auto rank: coloring) { values_per_rank[rank]++; }

        return values_per_rank;
    }

    inline auto get_global_index_by_rank(const std::vector<rank_id> &coloring, const int num_ranks)
            -> std::vector<int> {
        auto indices_per_rank = std::vector<std::vector<int>>(num_ranks);

        for (int i = 0; i < coloring.size(); i++) {
            const auto rank_id = coloring[i];
            indices_per_rank[rank_id].push_back(i);
        }

        auto indices_flat_view = indices_per_rank | std::views::join;
        return {indices_flat_view.begin(), indices_flat_view.end()};
    }

    template<typename Tc, std::size_t N>
    auto order_by_color(const std::span<Tc, N> values, const std::vector<rank_id> &coloring,
                        const std::vector<int> &displacements) {
        using T = std::remove_cv_t<Tc>;
        const int num_ranks = static_cast<int>(displacements.size());

        if (std::ranges::size(values) != coloring.size()) {
            throw std::invalid_argument("Length of coloring and values do not match");
        }

        auto ordered_values = std::vector<T>(std::ranges::size(values));
        auto num_sorted_per_rank = std::vector(num_ranks, 0);
        for (int i = 0; i < coloring.size(); ++i) {
            const auto dest_rank = coloring[i];
            const auto dest_index = displacements[dest_rank] + num_sorted_per_rank[dest_rank];
            ordered_values[dest_index] = values[i];

            num_sorted_per_rank[dest_rank]++;
        }

        return ordered_values;
    }
}// namespace reshuffle::internal

#endif//COLORING_UTILS_HPP
