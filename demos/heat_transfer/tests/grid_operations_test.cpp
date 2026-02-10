#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <grid_operations.hpp>

using namespace heat;
using namespace heat::internal;

using testing::Eq;

TEST(InitializeGrid, CreatesGridOfGivenDimentions) {
    const auto grid = initialize_grid(2, 3);
    const auto dimensions = get_dimensions(grid);

    EXPECT_THAT(dimensions, Eq(GridDimensions{2, 3}));
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

TEST(ToGrid, ConvertsOneDimensionalRepresentationToGrid) {
    const auto values = std::vector<double>{1, 2, 3, 4, 5, 6};
    const auto dimensions = reshuffle::Dimensions{2, 3};

    const auto expected = Matrix2D{{1, 2, 3}, {4, 5, 6}};
    EXPECT_THAT(to_grid(OneDimensionRepresentation{values, dimensions}).value(), Eq(expected));
}

TEST(ToGrid, ReturnsErrorIfMismatchedBetweenDimensionsAndNumValues) {
    const auto values = std::vector<double>{1, 2, 3};
    const auto dimensions = reshuffle::Dimensions{2, 4};

    EXPECT_THAT(to_grid(OneDimensionRepresentation{values, dimensions}).error(),
                Eq(ToGridError::MISMATCH_DIMENSIONS_AND_NUM_VALUES));
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
    constexpr auto expected = GridDimensions{2, 3};
    EXPECT_THAT(get_dimensions(grid), Eq(expected));
}

TEST(GetDimensions, WorksWithEmptyGrids) {
    constexpr auto grid = Matrix2D{};
    EXPECT_THAT(get_dimensions(grid), Eq(GridDimensions{0, 0}));
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

TEST(RemoveTopRowIfNecessary, RemovesTopRowOnlyIfThereIsANeighbour) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto processor_with_top_neighbour =
            ProcessorInfo{0, 1, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL};
    const auto processor_without_top_neighbour = ProcessorInfo{0, MPI_PROC_NULL, 1, 2, 3};

    EXPECT_THAT(remove_top_row_if_necessary(grid, processor_without_top_neighbour), Eq(grid));
    EXPECT_THAT(remove_top_row_if_necessary(grid, processor_with_top_neighbour),
                Eq(Matrix2D{{4, 5, 6}}));
}

TEST(LeaveTopRowIfNecessary, LeavesTopRowOnlyIfThereIsANeighbour) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto processor_with_top_neighbour =
            ProcessorInfo{0, 1, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL};
    const auto processor_without_top_neighbour = ProcessorInfo{0, MPI_PROC_NULL, 1, 2, 3};

    EXPECT_THAT(leave_top_row_if_necessary(grid, processor_with_top_neighbour), Eq(grid));
    EXPECT_THAT(leave_top_row_if_necessary(grid, processor_without_top_neighbour),
                Eq(Matrix2D{{4, 5, 6}}));
}

TEST(RemoveBottomRow, RemovesTheBottomRowOfAGrid) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = Matrix2D{{1, 2, 3}, {4, 5, 6}};
    EXPECT_THAT(remove_bottom_row(grid), Eq(expected));
}

TEST(RemoveBottomRowIfNecessary, RemovesBottomRowOnlyIfThereIsANeighbour) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto processor_with_bottom_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, 1, MPI_PROC_NULL, MPI_PROC_NULL};
    const auto processor_without_bottom_neighbour = ProcessorInfo{0, 1, MPI_PROC_NULL, 2, 3};

    EXPECT_THAT(remove_bottom_row_if_necessary(grid, processor_without_bottom_neighbour), Eq(grid));
    EXPECT_THAT(remove_bottom_row_if_necessary(grid, processor_with_bottom_neighbour),
                Eq(Matrix2D{{1, 2, 3}}));
}

TEST(LeaveBottomRowIfNecessary, LeavesBottomRowOnlyIfThereIsANeighbour) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto processor_with_bottom_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, 1, MPI_PROC_NULL, MPI_PROC_NULL};
    const auto processor_without_bottom_neighbour = ProcessorInfo{0, 1, MPI_PROC_NULL, 2, 3};

    EXPECT_THAT(leave_bottom_row_if_necessary(grid, processor_with_bottom_neighbour), Eq(grid));
    EXPECT_THAT(leave_bottom_row_if_necessary(grid, processor_without_bottom_neighbour),
                Eq(Matrix2D{{1, 2, 3}}));
}

TEST(RemoveLeftColumn, RemovesTheLeftColumnOfAGrid) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = Matrix2D{{2, 3}, {5, 6}, {8, 9}};
    EXPECT_THAT(remove_left_column(grid), Eq(expected));
}

TEST(RemoveLeftColumnIfNecessary, RemovesLeftColumnOnlyIfThereIsANeighbour) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto processor_with_left_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, MPI_PROC_NULL, 1, MPI_PROC_NULL};
    const auto processor_without_left_neighbour = ProcessorInfo{0, 1, 2, MPI_PROC_NULL, 3};

    EXPECT_THAT(remove_left_column_if_necessary(grid, processor_without_left_neighbour), Eq(grid));
    EXPECT_THAT(remove_left_column_if_necessary(grid, processor_with_left_neighbour),
                Eq(Matrix2D{{2, 3}, {5, 6}}));
}

TEST(LeaveLeftColumnIfNecessary, LeavesLeftColumnOnlyIfThereIsANeighbour) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto processor_with_left_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, MPI_PROC_NULL, 1, MPI_PROC_NULL};
    const auto processor_without_left_neighbour = ProcessorInfo{0, 1, 2, MPI_PROC_NULL, 3};

    EXPECT_THAT(leave_left_column_if_necessary(grid, processor_with_left_neighbour), Eq(grid));
    EXPECT_THAT(leave_left_column_if_necessary(grid, processor_without_left_neighbour),
                Eq(Matrix2D{{2, 3}, {5, 6}}));
}

TEST(RemoveRightColumn, RemovesTheRightColumnOfAGrid) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    const auto expected = Matrix2D{{1, 2}, {4, 5}, {7, 8}};
    EXPECT_THAT(remove_right_column(grid), Eq(expected));
}

TEST(RemoveRightColumnIfNecessary, RemovesRightColumnOnlyIfThereIsANeighbour) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto processor_with_right_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, 1};
    const auto processor_without_right_neighbour = ProcessorInfo{0, 1, 2, 3, MPI_PROC_NULL};

    EXPECT_THAT(remove_right_column_if_necessary(grid, processor_without_right_neighbour),
                Eq(grid));
    EXPECT_THAT(remove_right_column_if_necessary(grid, processor_with_right_neighbour),
                Eq(Matrix2D{{1, 2}, {4, 5}}));
}

TEST(LeaveRightColumnIfNecessary, LeavesRightColumnOnlyIfThereIsANeighbour) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto processor_with_right_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, 1};
    const auto processor_without_right_neighbour = ProcessorInfo{0, 1, 2, 3, MPI_PROC_NULL};

    EXPECT_THAT(leave_right_column_if_necessary(grid, processor_with_right_neighbour), Eq(grid));
    EXPECT_THAT(leave_right_column_if_necessary(grid, processor_without_right_neighbour),
                Eq(Matrix2D{{1, 2}, {4, 5}}));
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

TEST(AddGhostLayers, DoesNothingIfGridIsEmpty) {
    constexpr auto grid = Matrix2D{};
    EXPECT_TRUE(add_ghost_layers(grid).empty());
}

TEST(AddGhostLayers, AddsTopLayerOnlyIfProcessorHasANeighbour) {
    const auto grid = Matrix2D{{4, 5, 6}, {7, 8, 9}};

    const auto processor_with_top_neighbour =
            ProcessorInfo{0, 1, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL};
    const auto processor_without_top_neighbour = ProcessorInfo{0, MPI_PROC_NULL, 2, 3, 4};

    EXPECT_THAT(add_ghost_layers(grid, processor_with_top_neighbour),
                Eq(Matrix2D{{0, 0, 0}, {4, 5, 6}, {7, 8, 9}}));
    EXPECT_THAT(add_ghost_layers(grid, processor_without_top_neighbour),
                Eq(Matrix2D{{0, 4, 5, 6, 0}, {0, 7, 8, 9, 0}, {0, 0, 0, 0, 0}}));
}

TEST(AddGhostLayers, AddsBottomLayerOnlyIfProcessorHasANeighbour) {
    const auto grid = Matrix2D{{4, 5, 6}, {7, 8, 9}};

    const auto processor_with_bottom_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, 1, MPI_PROC_NULL, MPI_PROC_NULL};
    const auto processor_without_bottom_neighbour = ProcessorInfo{0, 1, MPI_PROC_NULL, 3, 4};

    EXPECT_THAT(add_ghost_layers(grid, processor_with_bottom_neighbour),
                Eq(Matrix2D{{4, 5, 6}, {7, 8, 9}, {0, 0, 0}}));
    EXPECT_THAT(add_ghost_layers(grid, processor_without_bottom_neighbour),
                Eq(Matrix2D{{0, 0, 0, 0, 0}, {0, 4, 5, 6, 0}, {0, 7, 8, 9, 0}}));
}

TEST(AddGhostLayers, AddsLeftLayerOnlyIfProcessorHasANeighbour) {
    const auto grid = Matrix2D{{4, 5, 6}, {7, 8, 9}};

    const auto processor_with_left_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, MPI_PROC_NULL, 1, MPI_PROC_NULL};
    const auto processor_without_left_neighbour = ProcessorInfo{0, 1, 2, MPI_PROC_NULL, 4};

    EXPECT_THAT(add_ghost_layers(grid, processor_with_left_neighbour),
                Eq(Matrix2D{{0, 4, 5, 6}, {0, 7, 8, 9}}));
    EXPECT_THAT(add_ghost_layers(grid, processor_without_left_neighbour),
                Eq(Matrix2D{{0, 0, 0, 0}, {4, 5, 6, 0}, {7, 8, 9, 0}, {0, 0, 0, 0}}));
}

TEST(AddGhostLayers, AddsRightLayerOnlyIfProcessorHasANeighbour) {
    const auto grid = Matrix2D{{4, 5, 6}, {7, 8, 9}};

    const auto processor_with_right_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, 1};
    const auto processor_without_right_neighbour = ProcessorInfo{0, 1, 2, 3, MPI_PROC_NULL};

    EXPECT_THAT(add_ghost_layers(grid, processor_with_right_neighbour),
                Eq(Matrix2D{{4, 5, 6, 0}, {7, 8, 9, 0}}));
    EXPECT_THAT(add_ghost_layers(grid, processor_without_right_neighbour),
                Eq(Matrix2D{{0, 0, 0, 0}, {0, 4, 5, 6}, {0, 7, 8, 9}, {0, 0, 0, 0}}));
}

TEST(RemoveGhostLayers, RemovesTopLayerOnlyIfProcessorHasANeighbour) {
    const auto grid = Matrix2D{{0, 0, 0, 0, 0}, {0, 4, 5, 6, 0}, {0, 7, 8, 9, 0}, {0, 0, 0, 0, 0}};

    const auto processor_with_top_neighbour =
            ProcessorInfo{0, 1, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL};
    const auto processor_without_top_neighbour = ProcessorInfo{0, MPI_PROC_NULL, 2, 3, 4};

    EXPECT_THAT(remove_ghost_layers(grid, processor_with_top_neighbour),
                Eq(Matrix2D{{0, 4, 5, 6, 0}, {0, 7, 8, 9, 0}, {0, 0, 0, 0, 0}}));
    EXPECT_THAT(remove_ghost_layers(grid, processor_without_top_neighbour),
                Eq(Matrix2D{{0, 0, 0}, {4, 5, 6}, {7, 8, 9}}));
}

TEST(RemoveGhostLayers, RemovesBottomLayerOnlyIfProcessorHasANeighbour) {
    const auto grid = Matrix2D{{0, 0, 0, 0, 0}, {0, 4, 5, 6, 0}, {0, 7, 8, 9, 0}, {0, 0, 0, 0, 0}};

    const auto processor_with_bottom_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, 1, MPI_PROC_NULL, MPI_PROC_NULL};
    const auto processor_without_bottom_neighbour = ProcessorInfo{0, 1, MPI_PROC_NULL, 3, 4};

    EXPECT_THAT(remove_ghost_layers(grid, processor_with_bottom_neighbour),
                Eq(Matrix2D{{0, 0, 0, 0, 0}, {0, 4, 5, 6, 0}, {0, 7, 8, 9, 0}}));
    EXPECT_THAT(remove_ghost_layers(grid, processor_without_bottom_neighbour),
                Eq(Matrix2D{{4, 5, 6}, {7, 8, 9}, {0, 0, 0}}));
}

TEST(RemoveGhostLayers, RemovesLeftLayerOnlyIfProcessorHasANeighbour) {
    const auto grid = Matrix2D{{0, 0, 0, 0, 0}, {0, 4, 5, 6, 0}, {0, 7, 8, 9, 0}, {0, 0, 0, 0, 0}};

    const auto processor_with_left_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, MPI_PROC_NULL, 1, MPI_PROC_NULL};
    const auto processor_without_left_neighbour = ProcessorInfo{0, 1, 2, MPI_PROC_NULL, 4};

    EXPECT_THAT(remove_ghost_layers(grid, processor_with_left_neighbour),
                Eq(Matrix2D{{0, 0, 0, 0}, {4, 5, 6, 0}, {7, 8, 9, 0}, {0, 0, 0, 0}}));
    EXPECT_THAT(remove_ghost_layers(grid, processor_without_left_neighbour),
                Eq(Matrix2D{{0, 4, 5, 6}, {0, 7, 8, 9}}));
}

TEST(RemoveGhostLayers, RemovesRightLayerOnlyIfProcessorHasANeighbour) {
    const auto grid = Matrix2D{{0, 0, 0, 0, 0}, {0, 4, 5, 6, 0}, {0, 7, 8, 9, 0}, {0, 0, 0, 0, 0}};

    const auto processor_with_right_neighbour =
            ProcessorInfo{0, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, 1};
    const auto processor_without_right_neighbour = ProcessorInfo{0, 1, 2, 3, MPI_PROC_NULL};

    EXPECT_THAT(remove_ghost_layers(grid, processor_with_right_neighbour),
                Eq(Matrix2D{{0, 0, 0, 0}, {0, 4, 5, 6}, {0, 7, 8, 9}, {0, 0, 0, 0}}));
    EXPECT_THAT(remove_ghost_layers(grid, processor_without_right_neighbour),
                Eq(Matrix2D{{4, 5, 6, 0}, {7, 8, 9, 0}}));
}

TEST(GetRow, ReturnsRowByIndex) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto row_index = 1;

    EXPECT_THAT(get_row(grid, row_index), Eq(std::vector<double>{4, 5, 6}));
}

TEST(GetRow, ReturnsErrorIfIndexOutOfBounds) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto index_out_of_bounds = grid.size();

    EXPECT_THAT(get_row(grid, index_out_of_bounds).error(), Eq(GetRowError::INDEX_OUT_OF_BOUNDS));
}

TEST(GetRow, ReturnsErrorIfGridIsEmtpy) {
    constexpr auto empty_grid = Matrix2D{};
    constexpr auto dummy_index = 0;

    EXPECT_THAT(get_row(empty_grid, dummy_index).error(), Eq(GetRowError::EMPTY_GRID));
}

TEST(GetColumn, ReturnsColumnByIndex) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto column_index = 1;

    EXPECT_THAT(get_column(grid, column_index).value(), Eq(std::vector<double>{2, 5, 8}));
}

TEST(GetColumn, ReturnsErrorIfIndexOutOfBounds) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto index_out_of_bounds = grid[0].size();

    EXPECT_THAT(get_column(grid, index_out_of_bounds).error(),
                Eq(GetColumnError::INDEX_OUT_OF_BOUNDS));
}

TEST(GetColumn, ReturnsErrorIfGridIsEmpty) {
    constexpr auto empty_grid = Matrix2D{};
    constexpr auto dummy_index = 0;

    EXPECT_THAT(get_column(empty_grid, dummy_index).error(), Eq(GetColumnError::EMPTY_GRID));
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

TEST(SetGhostLayer, CanSetValuesTopGhostLayer) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11, 12};

    const auto expected = Matrix2D{{10, 11, 12}, {4, 5, 6}, {7, 8, 9}};
    EXPECT_THAT(set_ghost_layer(grid, values, Location::TOP).value(), Eq(expected));
}

TEST(SetGhostLayer, CanSetValuesBottomGhostLayer) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11, 12};

    const auto expected = Matrix2D{{1, 2, 3}, {4, 5, 6}, {10, 11, 12}};
    EXPECT_THAT(set_ghost_layer(grid, values, Location::BOTTOM).value(), Eq(expected));
}

TEST(SetGhostLayer, CanSetValuesLeftGhostLayer) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11, 12};

    const auto expected = Matrix2D{{10, 2, 3}, {11, 5, 6}, {12, 8, 9}};
    EXPECT_THAT(set_ghost_layer(grid, values, Location::LEFT).value(), Eq(expected));
}

TEST(SetGhostLayer, CanSetValuesRightGhostLayer) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11, 12};

    const auto expected = Matrix2D{{1, 2, 10}, {4, 5, 11}, {7, 8, 12}};
    EXPECT_THAT(set_ghost_layer(grid, values, Location::RIGHT).value(), Eq(expected));
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

    EXPECT_THAT(set_top_row(grid, values).error(), Eq(SetBoundaryError::EMPTY_GRID));
}

TEST(SetTopRow, ReturnsErrorIfValuesIsEmptyButGridIsNot) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto values = std::vector<double>{};

    EXPECT_THAT(set_top_row(grid, values).error(), Eq(SetBoundaryError::EMPTY_VALUES));
}

TEST(SetTopRow, DoesNothingIfBothValuesAndGridAreEmtpy) {
    constexpr auto grid = Matrix2D{};
    constexpr auto values = std::vector<double>{};

    EXPECT_TRUE(set_top_row(grid, values).value().empty());
}

TEST(SetTopRow, ReturnsErrorIfNumValuesIsNotSameAsNumOfColumns) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};
    const auto values = std::vector<double>{10, 11};

    EXPECT_THAT(set_top_row(grid, values).error(), Eq(SetBoundaryError::INVALID_NUM_VALUES));
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

    EXPECT_THAT(set_bottom_row(grid, values).error(), Eq(SetBoundaryError::EMPTY_GRID));
}

TEST(SetBottomRow, ReturnsErrorIfValuesIsEmptyButGridIsNot) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto values = std::vector<double>{};

    EXPECT_THAT(set_bottom_row(grid, values).error(), Eq(SetBoundaryError::EMPTY_VALUES));
}

TEST(SetBottomRow, DoesNothingIfBothValuesAndGridAreEmtpy) {
    constexpr auto grid = Matrix2D{};
    constexpr auto values = std::vector<double>{};

    EXPECT_TRUE(set_bottom_row(grid, values).value().empty());
}

TEST(SetBottomRow, ReturnsErrorIfNumValuesIsNotSameAsNumOfColumns) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};
    const auto values = std::vector<double>{10, 11};

    EXPECT_THAT(set_bottom_row(grid, values).error(), Eq(SetBoundaryError::INVALID_NUM_VALUES));
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

    EXPECT_THAT(set_left_column(grid, values).error(), Eq(SetBoundaryError::EMPTY_GRID));
}

TEST(SetLeftColumn, ReturnsErrorIfValuesIsEmptyButGridIsNot) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto values = std::vector<double>{};

    EXPECT_THAT(set_left_column(grid, values).error(), Eq(SetBoundaryError::EMPTY_VALUES));
}

TEST(SetLeftColumn, DoesNothingIfBothValuesAndGridAreEmtpy) {
    constexpr auto grid = Matrix2D{};
    constexpr auto values = std::vector<double>{};

    EXPECT_TRUE(set_left_column(grid, values).value().empty());
}

TEST(SetLeftColumn, ReturnsErrorIfNumValuesIsNotSameAsNumOfRows) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};
    const auto values = std::vector<double>{10, 11, 12};

    EXPECT_THAT(set_left_column(grid, values).error(), Eq(SetBoundaryError::INVALID_NUM_VALUES));
}

TEST(SetRightColumn, SetsValuesGridRightColumn) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    const auto values = std::vector<double>{10, 11, 12};

    const auto expected = Matrix2D{{1, 2, 10}, {4, 5, 11}, {7, 8, 12}};
    EXPECT_THAT(set_right_column(grid, values).value(), Eq(expected));
}

TEST(SetRightColumn, ReturnsErrorIfGridIsEmptyButValuesAreNot) {
    constexpr auto grid = Matrix2D{};
    const auto values = std::vector<double>{10, 11, 12};

    EXPECT_THAT(set_right_column(grid, values).error(), Eq(SetBoundaryError::EMPTY_GRID));
}

TEST(SetRightColumn, ReturnsErrorIfValuesIsEmptyButGridIsNot) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    constexpr auto values = std::vector<double>{};

    EXPECT_THAT(set_right_column(grid, values).error(), Eq(SetBoundaryError::EMPTY_VALUES));
}

TEST(SetRightColumn, DoesNothingIfBothValuesAndGridAreEmtpy) {
    constexpr auto grid = Matrix2D{};
    constexpr auto values = std::vector<double>{};

    EXPECT_TRUE(set_right_column(grid, values).value().empty());
}

TEST(SetRightColumn, ReturnsErrorIfNumValuesIsNotSameAsNumOfRows) {
    const auto grid = Matrix2D{{1, 2, 3}, {4, 5, 6}};
    const auto values = std::vector<double>{10, 11, 12};

    EXPECT_THAT(set_right_column(grid, values).error(), Eq(SetBoundaryError::INVALID_NUM_VALUES));
}