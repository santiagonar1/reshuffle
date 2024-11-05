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
}// namespace reshuffle::internal

#endif//RESHUFFLE_UTILS_HPP
