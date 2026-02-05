#include "grid_operations.hpp"

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
    }// namespace internal

    auto initialize_grid(const unsigned int num_rows, const unsigned int num_columns) -> Matrix2D {
        const auto grid = Matrix2D(num_rows, std::vector<double>(num_columns, 0));
        return internal::set_boundary_to(grid, 100);
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

}// namespace heat