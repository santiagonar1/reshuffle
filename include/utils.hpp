#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <array>
#include <ranges>
#include <vector>

namespace reshuffle::internal {
    template<typename Tuple>
    auto unzip(const std::vector<Tuple> &tuples)
            -> std::array<std::vector<std::tuple_element_t<0, Tuple>>, std::tuple_size_v<Tuple>> {
        using FirstType = std::tuple_element_t<0, Tuple>;
        constexpr std::size_t tuple_size = std::tuple_size_v<Tuple>;
        auto result = std::array<std::vector<FirstType>, tuple_size>{};

        for (const auto &tuple: tuples) {
            std::apply(
                    [&result](const auto &...elements) {
                        std::size_t idx = 0;
                        ((result[idx++].push_back(elements)), ...);
                    },
                    tuple);
        }

        return result;
    }

    template<typename T>
    auto remove_duplicates(const std::vector<T> &values) -> std::vector<T> {
        auto unique_values = values;
        std::sort(unique_values.begin(), unique_values.end());
        auto [new_end, _] = std::ranges::unique(unique_values);
        unique_values.erase(new_end, unique_values.end());
        return unique_values;
    }
}// namespace reshuffle::internal

#endif//UTILS_HPP
