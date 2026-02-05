#ifndef RESHUFFLE_GRID_OPERATIONS_HPP
#define RESHUFFLE_GRID_OPERATIONS_HPP

#include "matrix.hpp"

namespace heat {
    enum class Location { LEFT, RIGHT, TOP, BOTTOM };

    namespace internal {
        auto set_boundary_to(const Matrix2D &grid, double value) -> Matrix2D;
        auto remove_top_row(const Matrix2D &grid) -> Matrix2D;
        auto remove_bottom_row(const Matrix2D &grid) -> Matrix2D;
        auto remove_left_column(const Matrix2D &grid) -> Matrix2D;
        auto remove_right_column(const Matrix2D &grid) -> Matrix2D;
    }// namespace internal

    auto initialize_grid(unsigned int num_rows, unsigned int num_columns) -> Matrix2D;
    auto get_dimensions(const Matrix2D &grid) -> std::pair<unsigned int, unsigned int>;
    auto apply_jacobi(const Matrix2D &grid) -> Matrix2D;
    auto add_ghost_layers(const Matrix2D &grid) -> Matrix2D;
    auto remove_ghost_layer(const Matrix2D &grid, const Location &location) -> Matrix2D;

}// namespace heat

#endif//RESHUFFLE_GRID_OPERATIONS_HPP
