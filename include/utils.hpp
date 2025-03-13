#ifndef RESHUFFLE_UTILS_HPP
#define RESHUFFLE_UTILS_HPP

#include <ranges>

#include "block_cyclic.hpp"
#include "concepts.hpp"
#include "coordinates.hpp"
#include "dimensions.hpp"

namespace reshuffle::internal {
    template<typename T>
    [[nodiscard]] auto cartesian_product(const std::vector<T> &first, const std::vector<T> &second)
            -> std::vector<std::pair<T, T>> {
        std::vector<std::pair<T, T>> combination{};
        for (const auto &v2: second) {
            for (const auto &v1: first) { combination.emplace_back(v1, v2); }
        }

        return combination;
    }

    template<typename T>
    [[nodiscard]] auto to_matrix(const std::vector<T> &values, const Dimension<2> &dimension)
            -> std::vector<std::vector<T>> {
        using Matrix = std::vector<std::vector<T>>;

        auto matrix = Matrix(dimension[1], std::vector<T>(dimension[0]));
        int i = 0;
        for (auto &row: matrix) {
            for (auto &value: row) {
                value = values[i];
                ++i;
            }
        }

        return matrix;
    }

    [[nodiscard]] auto get_2d_coordinates(int num_values_x, int index) -> Coordinates2D;

    template<concepts::Matrix2D M>
    [[nodiscard]] auto num_elements(const M &matrix) -> int {
        if (std::ranges::empty(matrix)) { return 0; }

        return std::ranges::size(matrix) * std::ranges::size(matrix[0]);
    }

    template<typename T>
    [[nodiscard]] auto get_values(const std::vector<T> &values, const std::vector<int> &indices)
            -> std::vector<T> {
        auto destiny_values = std::vector<T>(indices.size());

        for (int i = 0; i < indices.size(); i++) { destiny_values[i] = values.at(indices[i]); }

        return destiny_values;
    }

    template<typename T>
    [[nodiscard]] auto reorder_values(const std::vector<T> &values,
                                      const std::vector<int> &new_indices) -> std::vector<T> {
        if (values.size() != new_indices.size()) {
            throw std::invalid_argument("values.size() != indices.size()");
        }

        auto reordered_values = std::vector<T>(values.size());

        for (int i = 0; i < values.size(); i++) { reordered_values[new_indices[i]] = values[i]; }


        return reordered_values;
    }


    [[nodiscard]] auto have_same_num_values(const BlockCyclic &first, const BlockCyclic &second)
            -> bool;

    [[nodiscard]] auto have_same_num_values(const std::array<BlockCyclic, 2> &first,
                                            const std::array<BlockCyclic, 2> &second) -> bool;

    [[nodiscard]] auto num_values_in_rank(const std::array<BlockCyclic, 2> &distribution,
                                          rank_id rank) -> int;

    [[nodiscard]] auto num_ranks(const std::array<BlockCyclic, 2> &distribution) -> int;

    [[nodiscard]] auto get_num_repetitions(const std::vector<int> &values, int max_value)
            -> std::vector<int>;

    template<concepts::ContiguousContainer C>
    [[nodiscard]] auto group_values_by_rank_id(const C &values,
                                               const std::vector<rank_id> &associated_rank_ids,
                                               const int num_ranks)
            -> std::vector<typename C::value_type> {
        if (values.size() != associated_rank_ids.size()) {
            throw std::invalid_argument("values.size() != associated_rank_ids.size()");
        }

        const auto num_values_per_rank = get_num_repetitions(associated_rank_ids, num_ranks - 1);
        auto grouped_values = std::vector<int>(values.size());
        auto positions_by_rank = std::vector<int>(num_ranks);

        std::exclusive_scan(num_values_per_rank.begin(), num_values_per_rank.end(),
                            positions_by_rank.begin(), 0);
        for (int i = 0; i < associated_rank_ids.size(); i++) {
            const auto rank = associated_rank_ids[i];
            grouped_values[positions_by_rank[rank]] = values[i];
            positions_by_rank[rank]++;
        }

        return grouped_values;
    }
}// namespace reshuffle::internal

#endif//RESHUFFLE_UTILS_HPP
