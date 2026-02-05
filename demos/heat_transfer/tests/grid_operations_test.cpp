#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <grid_operations.hpp>

using namespace heat;
using namespace heat::internal;

using testing::Eq;

TEST(InitializeGrid, CreatesGridOfGivenDimentions) {
    const auto grid = initialize_grid(2, 3);
    const auto dimensions = get_dimensions(grid);

    EXPECT_THAT(dimensions, Eq(std::pair{2, 3}));
}

TEST(InitializeGrid, InitializesBoundaryTo100) {
    const auto grid = initialize_grid(2, 3);
    const auto [num_rows, num_columns] = get_dimensions(grid);

    for (auto i = 0; i < num_rows; i++) {
        EXPECT_THAT(grid[i][0], Eq(100));
        EXPECT_THAT(grid[i][num_columns - 1], Eq(100));
    }

    for (auto j = 0; j < num_columns; j++) {
        EXPECT_THAT(grid[0][j], Eq(100));
        EXPECT_THAT(grid[num_rows - 1][j], Eq(100));
    }
}

TEST(InitializeGrid, InitializesInnerCellsTo0) {
    const auto grid = initialize_grid(2, 3);
    const auto [num_rows, num_columns] = get_dimensions(grid);

    for (auto i = 1; i < num_rows - 1; i++) {
        for (auto j = 1; j < num_columns - 1; j++) { EXPECT_THAT(grid[i][j], Eq(0)); }
    }
}

TEST(SetBoundayTo, ReturnsNewGridWithNewBoundaryValues) {
    const auto values = Matrix2D{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};

    const auto result = set_boundary_to(values, 0);
    const auto expected = Matrix2D{{0, 0, 0, 0}, {0, 6, 7, 0}, {0, 10, 11, 0}, {0, 0, 0, 0}};
    EXPECT_THAT(result, Eq(expected));
}

TEST(GetDimensions, ReturnsGridDimensions) {
    const auto grid = initialize_grid(2, 3);
    constexpr auto expected = std::pair{2, 3};
    EXPECT_THAT(get_dimensions(grid), Eq(expected));
}

TEST(ApplyJacobi, AppliesJacobiMethodToGrid) {
    const auto grid = Matrix2D{{1, 1, 1, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 1, 1}};

    const auto expected = Matrix2D{{1, 1, 1, 1}, {1, 0.5, 0.5, 1}, {1, 0.5, 0.5, 1}, {1, 1, 1, 1}};
    const auto new_grid = apply_jacobi(grid);
    EXPECT_THAT(new_grid, Eq(expected));
}
