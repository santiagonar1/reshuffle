#ifndef CARTESIAN_PRODUCT_HPP
#define CARTESIAN_PRODUCT_HPP

#include <ranges>

#include "profiler.hpp"

namespace reshuffle::internal {

    template<typename T>
    [[nodiscard]] auto get_cartesian_product(const std::array<std::vector<T>, 1> &blocks)
            -> std::vector<std::array<T, 1>> {
        PROFILE_SCOPE_NAMED("get_cartesian_product");
        const auto vector = blocks | std::views::join;
        auto result = std::vector<std::array<T, 1>>{};

        for (const auto &block: vector) { result.emplace_back(std::array{block}); }

        return result;
    }

    template<typename T>
    [[nodiscard]] auto get_cartesian_product(const std::array<std::vector<T>, 2> &blocks)
            -> std::vector<std::array<T, 2>> {
        PROFILE_SCOPE_NAMED("get_cartesian_product");
        auto result = std::vector<std::array<T, 2>>{};

        for (const auto &v1: blocks[0]) {
            for (const auto &v2: blocks[1]) { result.emplace_back(std::array{v1, v2}); }
        }

        return result;
    }

    template<typename T>
    [[nodiscard]] auto get_cartesian_product(const std::array<std::vector<T>, 3> &blocks)
            -> std::vector<std::array<T, 3>> {
        PROFILE_SCOPE_NAMED("get_cartesian_product");
        auto result = std::vector<std::array<T, 3>>{};

        for (const auto &v1: blocks[0]) {
            for (const auto &v2: blocks[1]) {
                for (const auto &v3: blocks[2]) { result.emplace_back(std::array{v1, v2, v3}); }
            }
        }

        return result;
    }

}// namespace reshuffle::internal


#endif//CARTESIAN_PRODUCT_HPP
