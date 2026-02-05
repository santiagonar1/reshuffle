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

TEST(SetBoundaryTo, ReturnsNewGridWithNewBoundaryValues) {
    const auto values = Matrix2D{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};

    const auto result = set_boundary_to(values, 0);
    const auto expected = Matrix2D{{0, 0, 0, 0}, {0, 6, 7, 0}, {0, 10, 11, 0}, {0, 0, 0, 0}};
    EXPECT_THAT(result, Eq(expected));
}

TEST(SetBoundaryTo, DoesNothingIfGridIsEmtpy) {
    constexpr auto grid = Matrix2D{};
    EXPECT_TRUE(set_boundary_to(grid, 0).empty());
}

TEST(GetDimensions, ReturnsGridDimensions) {
    const auto grid = initialize_grid(2, 3);
    constexpr auto expected = std::pair{2, 3};
    EXPECT_THAT(get_dimensions(grid), Eq(expected));
}

TEST(GetDimensions, WorksWithEmptyGrids) {
    constexpr auto grid = Matrix2D{};
    EXPECT_THAT(get_dimensions(grid), Eq(std::pair{0, 0}));
}

TEST(ApplyJacobi, AppliesJacobiMethodToGrid) {
    const auto grid = Matrix2D{{1, 1, 1, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 1, 1}};

    const auto expected = Matrix2D{{1, 1, 1, 1}, {1, 0.5, 0.5, 1}, {1, 0.5, 0.5, 1}, {1, 1, 1, 1}};
    const auto new_grid = apply_jacobi(grid);
    EXPECT_THAT(new_grid, Eq(expected));
}

TEST(ApplyJacobi, DoesNothingIfGridIsEmtpy) {
    constexpr auto grid = Matrix2D{};
    EXPECT_TRUE(apply_jacobi(grid).empty());
}

TEST(RemoveTopRow, RemovesTheTopRowOfAGrid) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = Matrix2D{{4, 5, 6}, {7, 8, 9}};
    EXPECT_THAT(remove_top_row(grid), Eq(expected));
}

TEST(RemoveBottomRow, RemovesTheBottomRowOfAGrid) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = Matrix2D{{1, 2, 3}, {4, 5, 6}};
    EXPECT_THAT(remove_bottom_row(grid), Eq(expected));
}

TEST(RemoveLeftColumn, RemovesTheLeftColumnOfAGrid) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = Matrix2D{{2, 3}, {5, 6}, {8, 9}};
    EXPECT_THAT(remove_left_column(grid), Eq(expected));
}

TEST(RemoveRightColumn, RemovesTheRightColumnOfAGrid) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = Matrix2D{{1, 2}, {4, 5}, {7, 8}};
    EXPECT_THAT(remove_right_column(grid), Eq(expected));
}

TEST(AddGhostLayers, AddGhostLayersToGrid) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = Matrix2D{{0, 0, 0, 0, 0},
                                   {0, 1, 2, 3, 0},
                                   {0, 4, 5, 6, 0},
                                   {0, 7, 8, 9, 0},
                                   {0, 0, 0, 0, 0}};
    EXPECT_THAT(add_ghost_layers(grid), Eq(expected));
}

TEST(AddGhostLayer, DoesNothingIfGridIsEmpty) {
    constexpr auto grid = Matrix2D{};
    EXPECT_TRUE(add_ghost_layers(grid).empty());
}

TEST(RemoveGhostLayer, CanRemoveTopGhostLayer) {
    const auto grid = Matrix2D{{0, 0, 0, 0}, {0, 1, 2, 0}, {0, 3, 4, 0}, {0, 0, 0, 0}};

    const auto expected = Matrix2D{{0, 1, 2, 0}, {0, 3, 4, 0}, {0, 0, 0, 0}};
    EXPECT_THAT(remove_ghost_layer(grid, Location::TOP), Eq(expected));
}

TEST(RemoveGhostLayer, CanRemoveBottomGhostLayer) {
    const auto grid = Matrix2D{{0, 0, 0, 0}, {0, 1, 2, 0}, {0, 3, 4, 0}, {0, 0, 0, 0}};

    const auto expected = Matrix2D{{0, 0, 0, 0}, {0, 1, 2, 0}, {0, 3, 4, 0}};
    EXPECT_THAT(remove_ghost_layer(grid, Location::BOTTOM), Eq(expected));
}

TEST(RemoveGhostLayer, CanRemoveLeftGhostLayer) {
    const auto grid = Matrix2D{{0, 0, 0, 0}, {0, 1, 2, 0}, {0, 3, 4, 0}, {0, 0, 0, 0}};

    const auto expected = Matrix2D{{0, 0, 0}, {1, 2, 0}, {3, 4, 0}, {0, 0, 0}};
    EXPECT_THAT(remove_ghost_layer(grid, Location::LEFT), Eq(expected));
}

TEST(RemoveGhostLayer, CanRemoveRightGhostLayer) {
    const auto grid = Matrix2D{{0, 0, 0, 0}, {0, 1, 2, 0}, {0, 3, 4, 0}, {0, 0, 0, 0}};

    const auto expected = Matrix2D{{0, 0, 0}, {0, 1, 2}, {0, 3, 4}, {0, 0, 0}};
    EXPECT_THAT(remove_ghost_layer(grid, Location::RIGHT), Eq(expected));
}

TEST(RemoveGhostLayer, DoesNothingIfGridIsEmtpy) {
    constexpr auto grid = Matrix2D{};

    EXPECT_TRUE(remove_ghost_layer(grid, Location::TOP).empty());
    EXPECT_TRUE(remove_ghost_layer(grid, Location::BOTTOM).empty());
    EXPECT_TRUE(remove_ghost_layer(grid, Location::LEFT).empty());
    EXPECT_TRUE(remove_ghost_layer(grid, Location::RIGHT).empty());
}

TEST(GetTopRow, ReturnsGridTopRow) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = std::vector<double>{1, 2, 3};
    EXPECT_THAT(get_top_row(grid), Eq(expected));
}

TEST(GetBottomRow, ReturnsGridBottomRow) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = std::vector<double>{7, 8, 9};
    EXPECT_THAT(get_bottom_row(grid), Eq(expected));
}

TEST(GetLeftColumn, ReturnsGridLeftColumn) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = std::vector<double>{1, 4, 7};
    EXPECT_THAT(get_left_column(grid), Eq(expected));
}

TEST(GetRightColumn, ReturnsGridRightColumn) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = std::vector<double>{3, 6, 9};
    EXPECT_THAT(get_right_column(grid), Eq(expected));
}

TEST(GetGhostLayer, CanReturnTopGhostLayer) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = std::vector<double>{1, 2, 3};
    EXPECT_THAT(get_ghost_layer(grid, Location::TOP), Eq(expected));
}

TEST(GetGhostLayer, CanReturnBottomGhostLayer) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = std::vector<double>{7, 8, 9};
    EXPECT_THAT(get_ghost_layer(grid, Location::BOTTOM), Eq(expected));
}

TEST(GetGhostLayer, CanReturnLeftGhostLayer) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = std::vector<double>{1, 4, 7};
    EXPECT_THAT(get_ghost_layer(grid, Location::LEFT), Eq(expected));
}

TEST(GetGhostLayer, CanReturnRightGhostLayer) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = std::vector<double>{3, 6, 9};
    EXPECT_THAT(get_ghost_layer(grid, Location::RIGHT), Eq(expected));
}

TEST(GetGhostLayer, IfTheGridIsEmtpyReturnsEmtpyVector) {
    constexpr auto grid = Matrix2D{};

    EXPECT_TRUE(get_ghost_layer(grid, Location::TOP).empty());
    EXPECT_TRUE(get_ghost_layer(grid, Location::BOTTOM).empty());
    EXPECT_TRUE(get_ghost_layer(grid, Location::LEFT).empty());
    EXPECT_TRUE(get_ghost_layer(grid, Location::RIGHT).empty());
}

TEST(SetTopRow, SetsValuesGridTopRow) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11, 12};

    const auto expected = Matrix2D{{10, 11, 12}, {4, 5, 6}, {7, 8, 9}};
    EXPECT_THAT(set_top_row(grid, values).value(), Eq(expected));
}

TEST(SetTopRow, ReturnsErrorIfGridIsEmptyButValuesAreNot) {
    constexpr auto grid = Matrix2D{};
    const auto values = std::vector<double>{10, 11, 12};

    EXPECT_FALSE(set_top_row(grid, values).has_value());
}

TEST(SetTopRow, ReturnsErrorIfValuesIsEmptyButGridIsNot) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto values = std::vector<double>{};

    EXPECT_FALSE(set_top_row(grid, values).has_value());
}

TEST(SetTopRow, DoesNothingIfBothValuesAndGridAreEmtpy) {
    constexpr auto grid = Matrix2D{};
    constexpr auto values = std::vector<double>{};

    EXPECT_TRUE(set_top_row(grid, values).value().empty());
}

TEST(SetTopRow, ReturnsErrorIfNumValuesIsNotSameAsNumOfColumns) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11};

    EXPECT_FALSE(set_top_row(grid, values).has_value());
}

TEST(SetBottomRow, SetsValuesGridBottomRow) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11, 12};

    const auto expected = Matrix2D{{1, 2, 3}, {4, 5, 6}, {10, 11, 12}};
    EXPECT_THAT(set_bottom_row(grid, values).value(), Eq(expected));
}

TEST(SetBottomRow, ReturnsErrorIfGridIsEmptyButValuesAreNot) {
    constexpr auto grid = Matrix2D{};
    const auto values = std::vector<double>{10, 11, 12};

    EXPECT_FALSE(set_bottom_row(grid, values).has_value());
}

TEST(SetBottomRow, ReturnsErrorIfValuesIsEmptyButGridIsNot) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto values = std::vector<double>{};

    EXPECT_FALSE(set_bottom_row(grid, values).has_value());
}

TEST(SetBottomRow, DoesNothingIfBothValuesAndGridAreEmtpy) {
    constexpr auto grid = Matrix2D{};
    constexpr auto values = std::vector<double>{};

    EXPECT_TRUE(set_bottom_row(grid, values).value().empty());
}

TEST(SetBottomRow, ReturnsErrorIfNumValuesIsNotSameAsNumOfColumns) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11};

    EXPECT_FALSE(set_top_row(grid, values).has_value());
}

TEST(SetLeftColumn, SetsValuesGridLeftColumn) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11, 12};

    const auto expected = Matrix2D{{10, 2, 3}, {11, 5, 6}, {12, 8, 9}};
    EXPECT_THAT(set_left_column(grid, values).value(), Eq(expected));
}

TEST(SetLeftColumn, ReturnsErrorIfGridIsEmptyButValuesAreNot) {
    constexpr auto grid = Matrix2D{};
    const auto values = std::vector<double>{10, 11, 12};

    EXPECT_FALSE(set_left_column(grid, values).has_value());
}

TEST(SetLeftColumn, ReturnsErrorIfValuesIsEmptyButGridIsNot) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto values = std::vector<double>{};

    EXPECT_FALSE(set_left_column(grid, values).has_value());
}

TEST(SetLeftColumn, DoesNothingIfBothValuesAndGridAreEmtpy) {
    constexpr auto grid = Matrix2D{};
    constexpr auto values = std::vector<double>{};

    EXPECT_TRUE(set_left_column(grid, values).value().empty());
}

TEST(SetLeftColumn, ReturnsErrorIfNumValuesIsNotSameAsNumOfRows) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};
    const auto values = std::vector<double>{10, 11, 12};

    EXPECT_FALSE(set_left_column(grid, values).has_value());
}
