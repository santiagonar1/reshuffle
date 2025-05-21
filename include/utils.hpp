#ifndef UTILS_HPP
#define UTILS_HPP

#include <array>
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
}// namespace reshuffle::internal

#endif//UTILS_HPP
