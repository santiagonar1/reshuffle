#ifndef RESHUFFLE_UTILS_HPP
#define RESHUFFLE_UTILS_HPP

#include "block_cyclic.hpp"
#include "dimensions.hpp"

namespace reshuffle::internal {
    template<typename T>
    auto combine(const std::vector<T> &first, const std::vector<T> &second)
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

    auto have_same_num_values(const BlockCyclic &first, const BlockCyclic &second) -> bool;

    auto have_same_num_values(const std::array<BlockCyclic, 2> &first,
                              const std::array<BlockCyclic, 2> &second) -> bool;
}// namespace reshuffle::internal

#endif//RESHUFFLE_UTILS_HPP
