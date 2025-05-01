#ifndef RESHUFFLE_COORDINATES_HPP
#define RESHUFFLE_COORDINATES_HPP

#include "dimensions.hpp"

#include <ranges>

namespace reshuffle::internal {
    struct Coordinates2D {
        int x_coordinate{};
        int y_coordinate{};
    };

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
                                   const Dimensions<N> &dimensions) -> int {
        const auto start = static_cast<int>(coordinates.size()) - 1;

        int offset = 0;
        int stride = 1;

        // In a "normal" linear algebra fashion, when we have a 2x1 matrix, we have a matrix
        // with 2 rows and 1 column. Nonetheless, in this project, we want to use Dimensions to
        // indicate how many partitions there are in the respective dimension. So, Dimensions{2, 1}
        // is NOT a matrix with two rows and two columns, but a matrix with two columns (2 partitions
        // in x-axis), and 1 row (1 partition in y-axis). The algorithm below assumes the classic
        // linear algebra interpretation, which is why we need to reverse our dimension array
        // beforehand

        // Start from the rightmost coordinate (least significant)
        for (const auto reversed = dimensions | std::views::reverse;
             const auto &[coordinate, dimension]:
             std::views::zip(coordinates, reversed) | std::views::reverse) {
            if (coordinate == dimension) {
                throw std::invalid_argument("Coordinate should be smaller than the dimension");
            }
            offset += stride * coordinate;
            stride *= dimension;
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

        // In a "normal" linear algebra fashion, when we have a 2x1 matrix, we have a matrix
        // with 2 rows and 1 column. Nonetheless, in this project, we want to use Dimensions to
        // indicate how many partitions there are in the respective dimension. So, Dimensions{2, 1}
        // is NOT a matrix with two rows and two columns, but a matrix with two columns (2 partitions
        // in x-axis), and 1 row (1 partition in y-axis). The algorithm below assumes the classic
        // linear algebra interpretation, which is why we need to reverse our dimension array
        // beforehand
        Coordinates<N> coordinates{};
        // Start from the rightmost coordinate (least significant)
        for (const auto reversed = dimensions | std::views::reverse;
             std::tuple<int &, const int &> elements:
             std::views::zip(coordinates, reversed) | std::views::reverse) {
            auto &coordinate = std::get<0>(elements);
            const auto &dimension = std::get<1>(elements);
            coordinate = index % dimension;
            index /= dimension;
        }

        // Check if the index was too large for the given dimensions
        if (index > 0) {
            throw std::invalid_argument("Index exceeds the total size of the dimensions");
        }

        return coordinates;
    }

}// namespace reshuffle::internal

#endif//RESHUFFLE_COORDINATES_HPP
