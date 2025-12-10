#ifndef RESHUFFLE_COORDINATES_HPP
#define RESHUFFLE_COORDINATES_HPP

#include <ranges>

#include "dimensions.hpp"
#include "profiler.hpp"

namespace reshuffle {
    template<std::size_t N>
    using Coordinates = std::array<int, N>;

    namespace internal {
        template<std::size_t N>
        [[nodiscard]] constexpr auto get_invalid_coordinates() -> Coordinates<N> {
            Coordinates<N> invalid_coordinates{};
            for (int i = 0; i < N; ++i) { invalid_coordinates[i] = -1; }
            return invalid_coordinates;
        }

        template<std::size_t N>
        [[nodiscard]] auto map_indices(const Coordinates<N> &coordinates,
                                       const Dimensions<N> &dimensions) -> std::optional<int> {
            PROFILE_SCOPE_NAMED("map_indices");

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

            PROFILE_SCOPE_NAMED("map_index");

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
    }// namespace internal
}// namespace reshuffle

#endif//RESHUFFLE_COORDINATES_HPP
