#include "utils.hpp"

#include <ranges>

namespace reshuffle::internal {
    auto have_same_num_values(const BlockCyclic &first, const BlockCyclic &second) -> bool {
        return first.get_num_values() == second.get_num_values();
    }

    auto have_same_num_values(const std::array<BlockCyclic, 2> &first,
                              const std::array<BlockCyclic, 2> &second) -> bool {
        return have_same_num_values(first[0], second[0]) and
               have_same_num_values(first[1], second[1]);
    }

    auto number_of_values_in_rank(const std::array<BlockCyclic, 2> &distribution,
                                  const rank_id rank) -> int {

        if (rank < 0) { throw std::invalid_argument("rank cannot be negative"); }

        auto num_ranks_per_dimension =
                distribution |
                std::views::transform([](const BlockCyclic &d) { return d.get_num_ranks(); });
        const auto num_ranks = std::reduce(num_ranks_per_dimension.begin(),
                                           num_ranks_per_dimension.end(), 1, std::multiplies());

        if (rank >= num_ranks) { return 0; }

        auto values_per_dimension =
                distribution | std::views::transform([rank](const BlockCyclic &d) {
                    const auto num_values = d.get_num_values(rank);
                    return num_values == 0 ? d.get_num_values() : num_values;
                }) |
                std::views::filter([](const int num_values) { return num_values > 0; });

        if (values_per_dimension.empty()) { return 0; }

        return std::accumulate(values_per_dimension.begin(), values_per_dimension.end(), 1,
                               std::multiplies());
    }
}// namespace reshuffle::internal