#ifndef RESHUFFLE_GRID_OPERATIONS_HPP
#define RESHUFFLE_GRID_OPERATIONS_HPP

#include "matrix.hpp"
#include "processor_info.hpp"
#include "reshuffle.hpp"

#include <expected>

namespace heat {
    enum class Location { LEFT, RIGHT, TOP, BOTTOM };
    enum class SetBoundaryError { INVALID_NUM_VALUES, EMPTY_VALUES, EMPTY_GRID };
    enum class ToGridError { MISMATCH_DIMENSIONS_AND_NUM_VALUES };
    enum class GetRowError { INDEX_OUT_OF_BOUNDS, EMPTY_GRID };
    enum class GetColumnError { INDEX_OUT_OF_BOUNDS, EMPTY_GRID };

    struct GridDimensions {
        unsigned int num_rows;
        unsigned int num_columns;

        constexpr bool operator==(const GridDimensions &) const = default;
    };

    using OneDimensionRepresentation = std::pair<std::vector<double>, reshuffle::Dimensions<2>>;

    namespace internal {
        [[nodiscard]] auto set_boundary_to(const Matrix2D &grid, double value) -> Matrix2D;

        [[nodiscard]] auto remove_top_row(const Matrix2D &grid) -> Matrix2D;
        [[nodiscard]] auto remove_bottom_row(const Matrix2D &grid) -> Matrix2D;
        [[nodiscard]] auto remove_left_column(const Matrix2D &grid) -> Matrix2D;
        [[nodiscard]] auto remove_right_column(const Matrix2D &grid) -> Matrix2D;

        [[nodiscard]] auto remove_top_row_if_necessary(const Matrix2D &grid,
                                                       const ProcessorInfo &processor) -> Matrix2D;
        [[nodiscard]] auto remove_bottom_row_if_necessary(const Matrix2D &grid,
                                                          const ProcessorInfo &processor)
                -> Matrix2D;
        [[nodiscard]] auto remove_left_column_if_necessary(const Matrix2D &grid,
                                                           const ProcessorInfo &processor)
                -> Matrix2D;
        [[nodiscard]] auto remove_right_column_if_necessary(const Matrix2D &grid,
                                                            const ProcessorInfo &processor)
                -> Matrix2D;

        [[nodiscard]] auto leave_top_row_if_necessary(const Matrix2D &grid,
                                                      const ProcessorInfo &processor) -> Matrix2D;
        [[nodiscard]] auto leave_bottom_row_if_necessary(const Matrix2D &grid,
                                                         const ProcessorInfo &processor)
                -> Matrix2D;
        [[nodiscard]] auto leave_left_column_if_necessary(const Matrix2D &grid,
                                                          const ProcessorInfo &processor)
                -> Matrix2D;
        [[nodiscard]] auto leave_right_column_if_necessary(const Matrix2D &grid,
                                                           const ProcessorInfo &processor)
                -> Matrix2D;

        [[nodiscard]] auto get_row(const Matrix2D &grid, unsigned int row_index)
                -> std::expected<std::vector<double>, GetRowError>;
        [[nodiscard]] auto get_column(const Matrix2D &grid, unsigned int column_index)
                -> std::expected<std::vector<double>, GetColumnError>;

        [[nodiscard]] auto get_top_row(const Matrix2D &grid) -> std::vector<double>;
        [[nodiscard]] auto get_bottom_row(const Matrix2D &grid) -> std::vector<double>;
        [[nodiscard]] auto get_left_column(const Matrix2D &grid) -> std::vector<double>;
        [[nodiscard]] auto get_right_column(const Matrix2D &grid) -> std::vector<double>;

        [[nodiscard]] auto set_top_row(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError>;
        [[nodiscard]] auto set_bottom_row(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError>;
        [[nodiscard]] auto set_left_column(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError>;
        [[nodiscard]] auto set_right_column(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError>;

        [[nodiscard]] auto add_ghost_layers(const Matrix2D &grid) -> Matrix2D;
        [[nodiscard]] auto exchange_left_right_ghost_layers(const Matrix2D &grid,
                                                            const ProcessorInfo &processor,
                                                            MPI_Comm cartesian_comm) -> Matrix2D;
        [[nodiscard]] auto exchange_bottom_up_ghost_layers(const Matrix2D &grid,
                                                           const ProcessorInfo &processor,
                                                           MPI_Comm cartesian_comm) -> Matrix2D;
    }// namespace internal

    [[nodiscard]] auto initialize_grid(unsigned int num_rows, unsigned int num_columns) -> Matrix2D;
    [[nodiscard]] auto to_grid(const OneDimensionRepresentation &input)
            -> std::expected<Matrix2D, ToGridError>;

    [[nodiscard]] auto get_dimensions(const Matrix2D &grid) -> GridDimensions;
    [[nodiscard]] auto apply_jacobi(const Matrix2D &grid) -> Matrix2D;

    [[nodiscard]] auto add_ghost_layers(const Matrix2D &grid, const ProcessorInfo &processor)
            -> Matrix2D;
    [[nodiscard]] auto remove_ghost_layers(const Matrix2D &grid, const ProcessorInfo &processor)
            -> Matrix2D;
    [[nodiscard]] auto get_ghost_layer(const Matrix2D &grid, const Location &location)
            -> std::vector<double>;
    [[nodiscard]] auto set_ghost_layer(const Matrix2D &grid, const std::vector<double> &values,
                                       const Location &location)
            -> std::expected<Matrix2D, SetBoundaryError>;
    [[nodiscard]] auto exchange_ghost_layers(const Matrix2D &grid, const ProcessorInfo &processor,
                                             MPI_Comm cartesian_comm) -> Matrix2D;

}// namespace heat

#endif//RESHUFFLE_GRID_OPERATIONS_HPP
