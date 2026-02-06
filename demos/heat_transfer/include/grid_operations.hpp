#ifndef RESHUFFLE_GRID_OPERATIONS_HPP
#define RESHUFFLE_GRID_OPERATIONS_HPP

#include "matrix.hpp"
#include "reshuffle.hpp"

#include <expected>

namespace heat {
    enum class Location { LEFT, RIGHT, TOP, BOTTOM };
    enum class SetBoundaryError { INVALID_NUM_VALUES, EMPTY_VALUES, EMPTY_GRID };
    enum class ToGridError { MISMATCH_DIMENSIONS_AND_NUM_VALUES };

    using OneDimensionRepresentation = std::pair<std::vector<double>, reshuffle::Dimensions<2>>;

    namespace internal {
        auto set_boundary_to(const Matrix2D &grid, double value) -> Matrix2D;

        auto remove_top_row(const Matrix2D &grid) -> Matrix2D;
        auto remove_bottom_row(const Matrix2D &grid) -> Matrix2D;
        auto remove_left_column(const Matrix2D &grid) -> Matrix2D;
        auto remove_right_column(const Matrix2D &grid) -> Matrix2D;

        auto get_top_row(const Matrix2D &grid) -> std::vector<double>;
        auto get_bottom_row(const Matrix2D &grid) -> std::vector<double>;
        auto get_left_column(const Matrix2D &grid) -> std::vector<double>;
        auto get_right_column(const Matrix2D &grid) -> std::vector<double>;

        auto set_top_row(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError>;
        auto set_bottom_row(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError>;
        auto set_left_column(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError>;
        auto set_right_column(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError>;
    }// namespace internal

    auto initialize_grid(unsigned int num_rows, unsigned int num_columns) -> Matrix2D;
    auto to_grid(const OneDimensionRepresentation &input) -> std::expected<Matrix2D, ToGridError>;

    auto get_dimensions(const Matrix2D &grid) -> std::pair<unsigned int, unsigned int>;
    auto apply_jacobi(const Matrix2D &grid) -> Matrix2D;

    auto add_ghost_layers(const Matrix2D &grid) -> Matrix2D;
    auto remove_ghost_layer(const Matrix2D &grid, const Location &location) -> Matrix2D;
    auto get_ghost_layer(const Matrix2D &grid, const Location &location) -> std::vector<double>;
    auto set_ghost_layer(const Matrix2D &grid, const std::vector<double> &values,
                         const Location &location) -> std::expected<Matrix2D, SetBoundaryError>;

}// namespace heat

#endif//RESHUFFLE_GRID_OPERATIONS_HPP
