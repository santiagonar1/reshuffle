#ifndef CARTESIAN_PRODUCT_HPP
#define CARTESIAN_PRODUCT_HPP

#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>

namespace reshuffle::dev::internal {

    template<typename T>
    [[nodiscard]] auto get_cartesian_product(const std::array<std::vector<T>, 1> &blocks)
            -> std::vector<std::array<T, 1>> {
        const auto vector = blocks | std::views::join;
        auto result = std::vector<std::array<T, 1>>{};

        for (const auto &block: vector) { result.emplace_back(std::array{block}); }

        return result;
    }

    template<typename T>
    [[nodiscard]] auto get_cartesian_product(const std::array<std::vector<T>, 2> &blocks)
            -> std::vector<std::array<T, 2>> {
        auto result = std::vector<std::array<T, 2>>{};

        // We assume a row-major ordering through the code, which implies that we first move in the
        // x-axis (i.e., horizontal) than in the y-axis (vertical). Since we assume that the first
        // block corresponds to x and the second to y, we need to modify first v1 and then v2.
        for (const auto &v2: blocks[1]) {
            for (const auto &v1: blocks[0]) { result.emplace_back(std::array{v1, v2}); }
        }

        return result;
    }

    template<typename T>
    [[nodiscard]] auto get_cartesian_product(const std::array<std::vector<T>, 3> &blocks)
            -> std::vector<std::array<T, 3>> {
        auto result = std::vector<std::array<T, 3>>{};

        for (const auto &v3: blocks[2]) {
            for (const auto &v2: blocks[1]) {
                for (const auto &v1: blocks[0]) { result.emplace_back(std::array{v1, v2, v3}); }
            }
        }

        return result;
    }

}// namespace reshuffle::dev::internal


#endif//CARTESIAN_PRODUCT_HPP
