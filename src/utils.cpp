#include "utils.hpp"

#include <ranges>

namespace reshuffle::internal {
    auto have_same_num_values(const BlockCyclic &first, const BlockCyclic &second) -> bool {
        return first.get_num_total_values() == second.get_num_total_values();
    }

    auto have_same_num_values(const std::array<BlockCyclic, 2> &first,
                              const std::array<BlockCyclic, 2> &second) -> bool {
        return have_same_num_values(first[0], second[0]) and
               have_same_num_values(first[1], second[1]);
    }

    auto num_ranks(const std::array<BlockCyclic, 2> &distribution) -> int {
        auto num_ranks_per_dimension =
                distribution |
                std::views::transform([](const BlockCyclic &d) { return d.get_num_ranks(); });
        return std::reduce(num_ranks_per_dimension.begin(), num_ranks_per_dimension.end(), 1,
                           std::multiplies());
    }

    auto get_2d_coordinates(const int num_values_x, const int index) -> Coordinates2D {
        return {index % num_values_x, index / num_values_x};
    }

    auto num_values_in_rank(const std::array<BlockCyclic, 2> &distribution, const rank_id rank)
            -> int {

        if (rank < 0) { throw std::invalid_argument("rank cannot be negative"); }

        if (rank >= num_ranks(distribution)) { return 0; }

        const auto num_ranks_x = distribution[0].get_num_ranks();
        const auto [x_coordinates, y_coordinates] = get_2d_coordinates(num_ranks_x, rank);

        auto values_per_dimension =
                std::array{distribution[0].get_num_values_hold_by(x_coordinates),
                           distribution[1].get_num_values_hold_by(y_coordinates)};

        return std::accumulate(values_per_dimension.begin(), values_per_dimension.end(), 1,
                               std::multiplies());
    }

    auto get_num_repetitions(const std::vector<int> &values, const int max_value)
            -> std::vector<int> {
        auto num_repetitions = std::vector(max_value + 1, 0);
        for (const auto value: values) { num_repetitions[value]++; }
        return num_repetitions;
    }
}// namespace reshuffle::internal