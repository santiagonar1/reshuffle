#ifndef RESHUFFLE_COORDINATES_HPP
#define RESHUFFLE_COORDINATES_HPP

#include "dimensions.hpp"

namespace reshuffle::internal {
    struct Coordinates2D {
        int x_coordinate{};
        int y_coordinate{};
    };

    template<std::size_t N>
    using Coordinates = std::array<int, N>;

    template<std::size_t N>
    [[nodiscard]] auto map_indices(const Coordinates<N> &coordinates,
                                   const Dimensions<N> &dimensions) -> int {
        const auto start = static_cast<int>(coordinates.size()) - 1;

        int offset = 0;
        int stride = 1;

        // Start from the rightmost dimension (least significant)
        for (int i = start; i >= 0; --i) {
            if (coordinates[i] == dimensions[i]) {
                throw std::invalid_argument("Coordinate should be smaller than the dimension");
            }
            offset += stride * coordinates[i];
            stride *= dimensions[i];
        }

        return offset;
    }

    template<std::size_t N>
    [[nodiscard]] auto map_index(int index, const Dimensions<N> &dimensions) -> Coordinates<N> {

        if (index < 0) { throw std::invalid_argument("Index cannot be negative"); }

        const auto max_index = calc_total_num_values(dimensions) - 1;
        if (index > max_index) {
            throw std::invalid_argument("Index exceeds the total size of the dimensions");
        }


        Coordinates<N> coordinates{};
        // Start from the rightmost dimension (least significant)
        for (int i = N - 1; i >= 0; --i) {
            coordinates[i] = index % dimensions[i];
            index /= dimensions[i];
        }

        // Check if the index was too large for the given dimensions
        if (index > 0) {
            throw std::invalid_argument("Index exceeds the total size of the dimensions");
        }

        return coordinates;
    }

}// namespace reshuffle::internal

#endif//RESHUFFLE_COORDINATES_HPP
