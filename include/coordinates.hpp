#ifndef RESHUFFLE_COORDINATES_HPP
#define RESHUFFLE_COORDINATES_HPP

#include "dimensions.hpp"

#include <ranges>

namespace reshuffle::internal {
    template<std::size_t N>
    using Coordinates = std::array<int, N>;

    template<std::size_t N>
    [[nodiscard]] constexpr auto get_invalid_coordinates() -> Coordinates<N> {
        Coordinates<N> invalid_coordinates{};
        for (int i = 0; i < N; ++i) { invalid_coordinates[i] = -1; }
        return invalid_coordinates;
    }

    template<std::size_t N>
    [[nodiscard]] auto map_indices(const Coordinates<N> &coordinates,
                                   const Dimensions<N> &dimensions) -> std::optional<int> {
        const auto start = static_cast<int>(coordinates.size()) - 1;

        int offset = 0;
        int stride = 1;

        // Start from the rightmost coordinate (least significant)
        for (const auto &[coordinate, dimension]:
             std::views::zip(coordinates, dimensions) | std::views::reverse) {
            if (coordinate == dimension) { return std::nullopt; }
            offset += stride * coordinate;
            stride *= dimension;
        }

        return offset;
    }

    template<std::size_t N>
    [[nodiscard]] auto map_index(int index, const Dimensions<N> &dimensions)
            -> std::optional<Coordinates<N>> {

        if (index < 0) { return std::nullopt; }

        const auto max_index = calc_total_num_values(dimensions) - 1;
        if (index > max_index) { return std::nullopt; }

        Coordinates<N> coordinates{};
        // Start from the rightmost coordinate (least significant)
        for (std::tuple<int &, const int &> elements:
             std::views::zip(coordinates, dimensions) | std::views::reverse) {
            auto &coordinate = std::get<0>(elements);
            const auto &dimension = std::get<1>(elements);
            coordinate = index % dimension;
            index /= dimension;
        }

        return coordinates;
    }

}// namespace reshuffle::internal

#endif//RESHUFFLE_COORDINATES_HPP
