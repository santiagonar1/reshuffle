#include "grid_operations.hpp"

#include <ranges>

#include <mdspan.hpp>

namespace heat {
    namespace internal {
        auto set_boundary_to(const Matrix2D &grid, const double value) -> Matrix2D {
            if (grid.empty()) { return {}; }

            auto result = grid;
            const auto num_rows = grid.size();
            const auto num_columns = grid[0].size();

            for (auto i = 0; i < num_rows; i++) {
                result[i][0] = value;
                result[i][num_columns - 1] = value;
            }

            for (auto j = 0; j < num_columns; j++) {
                result[0][j] = value;
                result[num_rows - 1][j] = value;
            }

            return result;
        }

        auto remove_top_row(const Matrix2D &grid) -> Matrix2D {
            if (grid.empty()) { return {}; }

            auto new_grid = Matrix2D{};
            for (const auto &row: grid | std::ranges::views::drop(1)) {
                new_grid.emplace_back(row);
            }

            return new_grid;
        }

        auto remove_top_row_if_necessary(const Matrix2D &grid, const ProcessorInfo &processor)
                -> Matrix2D {
            if (not processor.has_up_neighbour()) { return grid; }

            return remove_top_row(grid);
        }

        auto leave_top_row_if_necessary(const Matrix2D &grid, const ProcessorInfo &processor)
                -> Matrix2D {
            if (processor.has_up_neighbour()) { return grid; }

            return remove_top_row(grid);
        }

        auto remove_bottom_row(const Matrix2D &grid) -> Matrix2D {
            if (grid.empty()) { return {}; }

            auto new_grid = Matrix2D{};
            for (const auto &row: grid | std::ranges::views::take(grid.size() - 1)) {
                new_grid.emplace_back(row);
            }

            return new_grid;
        }

        auto remove_bottom_row_if_necessary(const Matrix2D &grid, const ProcessorInfo &processor)
                -> Matrix2D {
            if (not processor.has_down_neighbour()) { return grid; }

            return remove_bottom_row(grid);
        }

        auto leave_bottom_row_if_necessary(const Matrix2D &grid, const ProcessorInfo &processor)
                -> Matrix2D {
            if (processor.has_down_neighbour()) { return grid; }

            return remove_bottom_row(grid);
        }

        auto remove_left_column(const Matrix2D &grid) -> Matrix2D {
            if (grid.empty()) { return {}; }

            auto new_grid = Matrix2D{};
            for (const auto &row: grid) {
                auto row_without_left_column = row | std::ranges::views::drop(1);
                new_grid.emplace_back(row_without_left_column.begin(),
                                      row_without_left_column.end());
            }

            return new_grid;
        }

        auto remove_left_column_if_necessary(const Matrix2D &grid, const ProcessorInfo &processor)
                -> Matrix2D {
            if (not processor.has_left_neighbour()) { return grid; }

            return remove_left_column(grid);
        }

        auto leave_left_column_if_necessary(const Matrix2D &grid, const ProcessorInfo &processor)
                -> Matrix2D {
            if (processor.has_left_neighbour()) { return grid; }

            return remove_left_column(grid);
        }

        auto remove_right_column(const Matrix2D &grid) -> Matrix2D {
            if (grid.empty()) { return {}; }

            auto new_grid = Matrix2D{};
            for (const auto &row: grid) {
                auto row_without_right_column = row | std::ranges::views::take(row.size() - 1);
                new_grid.emplace_back(row_without_right_column.begin(),
                                      row_without_right_column.end());
            }

            return new_grid;
        }

        auto remove_right_column_if_necessary(const Matrix2D &grid, const ProcessorInfo &processor)
                -> Matrix2D {
            if (not processor.has_right_neighbour()) { return grid; }

            return remove_right_column(grid);
        }

        auto leave_right_column_if_necessary(const Matrix2D &grid, const ProcessorInfo &processor)
                -> Matrix2D {
            if (processor.has_right_neighbour()) { return grid; }

            return remove_right_column(grid);
        }

        auto get_row(const Matrix2D &grid, const unsigned int row_index)
                -> std::expected<std::vector<double>, GetRowError> {
            if (grid.empty()) { return std::unexpected(GetRowError::EMPTY_GRID); }

            if (row_index >= grid.size()) {
                return std::unexpected(GetRowError::INDEX_OUT_OF_BOUNDS);
            }

            return grid[row_index];
        }

        auto get_top_row(const Matrix2D &grid) -> std::vector<double> {
            if (grid.empty()) { return {}; }

            return grid.front();
        }

        auto get_bottom_row(const Matrix2D &grid) -> std::vector<double> {
            if (grid.empty()) { return {}; }

            return grid.back();
        }

        auto get_left_column(const Matrix2D &grid) -> std::vector<double> {
            if (grid.empty()) { return {}; }

            auto column = std::vector<double>{};
            for (const auto &row: grid) { column.emplace_back(row.front()); }

            return column;
        }

        auto get_right_column(const Matrix2D &grid) -> std::vector<double> {
            if (grid.empty()) { return {}; }

            auto column = std::vector<double>{};
            for (const auto &row: grid) { column.emplace_back(row.back()); }

            return column;
        }

        auto do_checks(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<void, SetBoundaryError> {
            if (grid.empty() and not values.empty()) {
                return std::unexpected(SetBoundaryError::EMPTY_GRID);
            }

            if (values.empty() and not grid.empty()) {
                return std::unexpected(SetBoundaryError::EMPTY_VALUES);
            }

            return {};
        }

        auto set_top_row(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError> {
            if (not do_checks(grid, values).has_value()) {
                return std::unexpected(do_checks(grid, values).error());
            }

            if (grid.empty() and values.empty()) { return {}; }

            if (const auto [num_rows, num_columns] = get_dimensions(grid);
                num_columns != values.size()) {
                return std::unexpected(SetBoundaryError::INVALID_NUM_VALUES);
            }

            auto new_grid = Matrix2D{};
            new_grid.emplace_back(values);

            for (const auto &row: grid | std::ranges::views::drop(1)) {
                new_grid.emplace_back(row);
            }

            return new_grid;
        }

        auto set_bottom_row(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError> {
            if (not do_checks(grid, values).has_value()) {
                return std::unexpected(do_checks(grid, values).error());
            }

            if (grid.empty() and values.empty()) { return {}; }

            if (const auto [num_rows, num_columns] = get_dimensions(grid);
                num_columns != values.size()) {
                return std::unexpected(SetBoundaryError::INVALID_NUM_VALUES);
            }

            auto new_grid = Matrix2D{};
            for (const auto &row: grid | std::ranges::views::take(grid.size() - 1)) {
                new_grid.emplace_back(row);
            }

            new_grid.emplace_back(values);

            return new_grid;
        }

        auto set_left_column(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError> {
            if (not do_checks(grid, values).has_value()) {
                return std::unexpected(do_checks(grid, values).error());
            }

            if (grid.empty() and values.empty()) { return {}; }

            const auto [num_rows, num_columns] = get_dimensions(grid);
            if (num_rows != values.size()) {
                return std::unexpected(SetBoundaryError::INVALID_NUM_VALUES);
            }

            auto new_grid = Matrix2D{};
            for (auto i = 0; i < num_rows; i++) {
                auto new_row = std::vector<double>(num_columns);
                new_row[0] = values[i];
                auto row_without_left_column = grid[i] | std::ranges::views::drop(1);
                std::ranges::copy(row_without_left_column, new_row.begin() + 1);
                new_grid.emplace_back(new_row);
            }

            return new_grid;
        }

        auto set_right_column(const Matrix2D &grid, const std::vector<double> &values)
                -> std::expected<Matrix2D, SetBoundaryError> {
            if (not do_checks(grid, values).has_value()) {
                return std::unexpected(do_checks(grid, values).error());
            }

            if (grid.empty() and values.empty()) { return {}; }

            const auto [num_rows, num_columns] = get_dimensions(grid);
            if (num_rows != values.size()) {
                return std::unexpected(SetBoundaryError::INVALID_NUM_VALUES);
            }

            auto new_grid = Matrix2D{};
            for (auto i = 0; i < num_rows; i++) {
                auto row_without_right_column = grid[i] | std::ranges::views::take(num_columns - 1);
                auto new_row = std::vector<double>{row_without_right_column.begin(),
                                                   row_without_right_column.end()};
                new_row.push_back(values[i]);
                new_grid.emplace_back(new_row);
            }

            return new_grid;
        }

        auto add_ghost_layers(const Matrix2D &grid) -> Matrix2D {
            if (grid.empty()) { return {}; }

            const auto [num_rows, num_columns] = get_dimensions(grid);

            auto new_grid = Matrix2D(num_rows + 2, std::vector<double>(num_columns + 2, 0));
            for (int i = 1; i < num_rows + 1; i++) {
                for (int j = 1; j < num_columns + 1; j++) { new_grid[i][j] = grid[i - 1][j - 1]; }
            }
            return new_grid;
        }

        auto exchange_left_right_ghost_layers(const Matrix2D &grid, const ProcessorInfo &processor,
                                              MPI_Comm cartesian_comm) -> Matrix2D {
            const auto [num_rows, _] = get_dimensions(grid);

            auto new_left_layer = processor.has_left_neighbour() ? std::vector<double>(num_rows)
                                                                 : std::vector<double>{};
            auto new_right_layer = processor.has_right_neighbour() ? std::vector<double>(num_rows)
                                                                   : std::vector<double>{};

            auto requests = std::array<MPI_Request, 4>{};
            auto n = 0;

            const auto left_layer = processor.has_left_neighbour()
                                            ? get_ghost_layer(grid, Location::LEFT)
                                            : std::vector<double>{};
            const auto right_layer = processor.has_right_neighbour()
                                             ? get_ghost_layer(grid, Location::RIGHT)
                                             : std::vector<double>{};

            MPI_Irecv(new_left_layer.data(), new_left_layer.size(), MPI_DOUBLE,
                      processor.get_left_neighbour(), 0, MPI_COMM_WORLD, &requests[n++]);
            MPI_Irecv(new_right_layer.data(), new_right_layer.size(), MPI_DOUBLE,
                      processor.get_right_neighbour(), 0, MPI_COMM_WORLD, &requests[n++]);
            MPI_Isend(left_layer.data(), left_layer.size(), MPI_DOUBLE,
                      processor.get_left_neighbour(), 0, MPI_COMM_WORLD, &requests[n++]);
            MPI_Isend(right_layer.data(), right_layer.size(), MPI_DOUBLE,
                      processor.get_right_neighbour(), 0, MPI_COMM_WORLD, &requests[n++]);

            MPI_Waitall(n, requests.data(), MPI_STATUSES_IGNORE);

            auto new_grid = grid;

            if (processor.has_left_neighbour()) {
                new_grid = set_ghost_layer(new_grid, new_left_layer, Location::LEFT).value();
            }

            if (processor.has_right_neighbour()) {
                new_grid = set_ghost_layer(grid, new_right_layer, Location::RIGHT).value();
            }

            return new_grid;
        }

        auto exchange_bottom_up_ghost_layers(const Matrix2D &grid, const ProcessorInfo &processor,
                                             MPI_Comm cartesian_comm) -> Matrix2D {

            const auto [num_rows, num_columns] = get_dimensions(grid);

            auto new_up_layer = processor.has_up_neighbour() ? std::vector<double>(num_columns)
                                                             : std::vector<double>{};
            auto new_down_layer = processor.has_down_neighbour() ? std::vector<double>(num_columns)
                                                                 : std::vector<double>{};

            auto requests = std::array<MPI_Request, 4>{};
            auto n = 0;

            const auto up_layer = processor.has_up_neighbour()
                                          ? get_ghost_layer(grid, Location::TOP)
                                          : std::vector<double>{};
            const auto down_layer = processor.has_down_neighbour()
                                            ? get_ghost_layer(grid, Location::BOTTOM)
                                            : std::vector<double>{};

            MPI_Irecv(new_up_layer.data(), new_up_layer.size(), MPI_DOUBLE,
                      processor.get_up_neighbour(), 0, MPI_COMM_WORLD, &requests[n++]);
            MPI_Irecv(new_down_layer.data(), new_down_layer.size(), MPI_DOUBLE,
                      processor.get_down_neighbour(), 0, MPI_COMM_WORLD, &requests[n++]);


            MPI_Isend(up_layer.data(), up_layer.size(), MPI_DOUBLE, processor.get_up_neighbour(), 0,
                      MPI_COMM_WORLD, &requests[n++]);
            MPI_Isend(down_layer.data(), down_layer.size(), MPI_DOUBLE,
                      processor.get_down_neighbour(), 0, MPI_COMM_WORLD, &requests[n++]);

            MPI_Waitall(n, requests.data(), MPI_STATUSES_IGNORE);

            auto new_grid = grid;

            if (processor.has_up_neighbour()) {
                new_grid = set_ghost_layer(new_grid, new_up_layer, Location::TOP).value();
            }

            if (processor.has_down_neighbour()) {
                new_grid = set_ghost_layer(grid, new_down_layer, Location::BOTTOM).value();
            }

            return new_grid;
        }
    }// namespace internal

    auto initialize_grid(const unsigned int num_rows, const unsigned int num_columns) -> Matrix2D {
        const auto grid = Matrix2D(num_rows, std::vector<double>(num_columns, 0));
        return internal::set_boundary_to(grid, 100);
    }

    auto to_grid(const OneDimensionRepresentation &input) -> std::expected<Matrix2D, ToGridError> {
        const auto &[values, dimensions] = input;

        if (reshuffle::internal::calc_total_num_values(dimensions) != values.size()) {
            return std::unexpected(ToGridError::MISMATCH_DIMENSIONS_AND_NUM_VALUES);
        }

        const auto as_span = std::mdspan(values.data(), dimensions);

        auto grid = Matrix2D(dimensions[0], std::vector<double>(dimensions[1], 0));
        for (int i = 0; i < dimensions[0]; i++) {
            for (int j = 0; j < dimensions[1]; j++) { grid[i][j] = as_span[i, j]; }
        }

        return grid;
    }

    auto get_dimensions(const Matrix2D &grid) -> std::pair<unsigned int, unsigned int> {
        if (grid.empty()) { return {}; }

        return {grid.size(), grid[0].size()};
    }

    auto apply_jacobi(const Matrix2D &grid) -> Matrix2D {
        if (grid.empty()) { return {}; }

        auto new_grid = grid;
        const auto [num_rows, num_columns] = get_dimensions(grid);

        for (int i = 1; i < num_rows - 1; i++) {
            for (int j = 1; j < num_columns - 1; j++) {
                new_grid[i][j] =
                        0.25 * (grid[i - 1][j] + grid[i + 1][j] + grid[i][j - 1] + grid[i][j + 1]);
            }
        }
        return new_grid;
    }

    auto add_ghost_layers(const Matrix2D &grid, const ProcessorInfo &processor) -> Matrix2D {
        using namespace internal;

        return leave_top_row_if_necessary(
                leave_bottom_row_if_necessary(
                        leave_left_column_if_necessary(
                                leave_right_column_if_necessary(add_ghost_layers(grid), processor),
                                processor),
                        processor),
                processor);
    }

    auto remove_ghost_layers(const Matrix2D &grid, const ProcessorInfo &processor) -> Matrix2D {
        using namespace internal;

        return remove_top_row_if_necessary(
                remove_bottom_row_if_necessary(
                        remove_left_column_if_necessary(
                                remove_right_column_if_necessary(grid, processor), processor),
                        processor),
                processor);
    }

    auto get_ghost_layer(const Matrix2D &grid, const Location &location) -> std::vector<double> {
        switch (location) {
            case Location::TOP:
                return internal::get_top_row(grid);
            case Location::BOTTOM:
                return internal::get_bottom_row(grid);
            case Location::LEFT:
                return internal::get_left_column(grid);
            case Location::RIGHT:
                return internal::get_right_column(grid);
        }

        throw std::runtime_error("Invalid location");
    }

    auto set_ghost_layer(const Matrix2D &grid, const std::vector<double> &values,
                         const Location &location) -> std::expected<Matrix2D, SetBoundaryError> {
        switch (location) {
            case Location::TOP:
                return internal::set_top_row(grid, values);
            case Location::BOTTOM:
                return internal::set_bottom_row(grid, values);
            case Location::LEFT:
                return internal::set_left_column(grid, values);
            case Location::RIGHT:
                return internal::set_right_column(grid, values);
        }

        throw std::runtime_error("Invalid location");
    }

    auto exchange_ghost_layers(const Matrix2D &grid, const ProcessorInfo &processor,
                               const MPI_Comm cartesian_comm) -> Matrix2D {
        return internal::exchange_bottom_up_ghost_layers(
                internal::exchange_left_right_ghost_layers(grid, processor, cartesian_comm),
                processor, cartesian_comm);
    }

}// namespace heat