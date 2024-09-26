#ifndef RESHUFFLE_UTILS_HPP
#define RESHUFFLE_UTILS_HPP

#include <ranges>

#include "block_cyclic.hpp"
#include "concepts.hpp"
#include "dimensions.hpp"
#include "indices.hpp"

namespace reshuffle::internal {
    template<typename T>
    auto cartesian_product(const std::vector<T> &first, const std::vector<T> &second)
            -> std::vector<std::pair<T, T>> {
        std::vector<std::pair<T, T>> combination{};
        for (const auto &v2: second) {
            for (const auto &v1: first) { combination.emplace_back(v1, v2); }
        }

        return combination;
    }

    template<typename T>
    auto to_matrix(const std::vector<T> &values, const Dimension<2> &dimension)
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

    auto to_2D(int num_columns, int index) -> Indices2D;

    template<concepts::Matrix2D M>
    auto num_elements(const M &matrix) -> int {
        if (std::ranges::empty(matrix)) { return 0; }

        return std::ranges::size(matrix) * std::ranges::size(matrix[0]);
    }


    [[nodiscard]] auto have_same_num_values(const BlockCyclic &first, const BlockCyclic &second)
            -> bool;

    [[nodiscard]] auto have_same_num_values(const std::array<BlockCyclic, 2> &first,
                                            const std::array<BlockCyclic, 2> &second) -> bool;

    [[nodiscard]] auto num_values_in_rank(const std::array<BlockCyclic, 2> &distribution,
                                          rank_id rank) -> int;

    [[nodiscard]] auto num_ranks(const std::array<BlockCyclic, 2> &distribution) -> int;
}// namespace reshuffle::internal

#endif//RESHUFFLE_UTILS_HPP
