#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <grid_layout.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::Eq;

auto get_blocks(int num_blocks) -> std::vector<Block>;

TEST(GridLayout, StoresBlocks) {
    const auto blocks = get_blocks(2);

    const auto grid_layout = GridLayout(std::array{blocks});

    EXPECT_THAT(grid_layout.get_blocks(), Eq(std::array{blocks}));
}

TEST(GridLayout, WorksIn2D) {
    const auto x_blocks = get_blocks(2);
    const auto y_blocks = get_blocks(2);

    const auto grid_layout = GridLayout(std::array{x_blocks, y_blocks});

    EXPECT_THAT(grid_layout.get_blocks(), Eq(std::array{x_blocks, y_blocks}));
}

TEST(GridLayout, CanReturnTheOwnerOfABlock) {
    constexpr auto num_processors = 2;

    const auto processor_grid = ProcessorGrid<1>{{num_processors}};
    const auto blocks = get_blocks(num_processors);
    const auto grid_layout = GridLayout(std::array{blocks});

    EXPECT_THAT(grid_layout.get_block_owner(Coordinates{0}, processor_grid), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates{1}, processor_grid), Eq(1));
}

TEST(GridLayout, CanReturnTheOwnerOfABlockIn2DVerticalPartition) {
    constexpr auto num_processors_x = 2;
    constexpr auto num_processors_y = 1;

    const auto processor_grid = ProcessorGrid<2>{{num_processors_y, num_processors_x}};
    const auto x_blocks = get_blocks(num_processors_x);
    const auto y_blocks = get_blocks(num_processors_y);

    const auto grid_layout = GridLayout(std::array{x_blocks, y_blocks});

    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 0}, processor_grid), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 1}, processor_grid), Eq(1));
}

TEST(GridLayout, CanReturnTheOwnerOfABlockIn2DHorizontalPartition) {
    constexpr auto num_processors_x = 1;
    constexpr auto num_processors_y = 2;

    const auto processor_grid = ProcessorGrid<2>{{num_processors_y, num_processors_x}};
    const auto x_blocks = get_blocks(num_processors_x);
    const auto y_blocks = get_blocks(num_processors_y);

    const auto grid_layout = GridLayout(std::array{x_blocks, y_blocks});

    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 0}, processor_grid), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{1, 0}, processor_grid), Eq(1));
}

TEST(GridLayout, CanReturnTheOwnerOfABlockIn2DCrossPartition) {
    constexpr auto num_processors_x = 2;
    constexpr auto num_processors_y = 2;

    const auto processor_grid = ProcessorGrid<2>{{num_processors_y, num_processors_x}};
    const auto x_blocks = get_blocks(num_processors_x);
    const auto y_blocks = get_blocks(num_processors_y);

    const auto grid_layout = GridLayout(std::array{x_blocks, y_blocks});

    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 0}, processor_grid), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 1}, processor_grid), Eq(1));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{1, 0}, processor_grid), Eq(2));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{1, 1}, processor_grid), Eq(3));
}

TEST(GridLayout, ThrowsWhenAskedForABlockIdOutOfRange) {
    constexpr auto num_processors = 2;

    const auto processor_grid = ProcessorGrid<1>{{num_processors}};
    const auto blocks = get_blocks(num_processors);
    const auto grid_layout = GridLayout(std::array{blocks});

    constexpr auto block_id_out_of_range = num_processors;

    EXPECT_THROW(auto _ = grid_layout.get_block_owner(Coordinates{block_id_out_of_range},
                                                      processor_grid),
                 std::out_of_range);
}

TEST(GridLayout, CalculatesTheOverlayOfTwoGrids) {
    const auto processor_grid = ProcessorGrid<1>{{2}};

    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto grid = GridLayout(std::array{origin_blocks});

    const auto target_blocks = std::vector{{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    const auto overlay = grid.get_overlay(target_grid, processor_grid);
    const auto &grid_overlay = overlay.get_grid();
    const auto &owners_target_grid = overlay.get_owners_target_grid();

    const auto expected_blocks_overlay =
            std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 0}, Block{{2, 4}, 1}};
    const auto expected_owners_target_grid = std::vector{std::vector{0, 1, 1}};

    EXPECT_THAT(grid_overlay.get_blocks(), Eq(std::array{expected_blocks_overlay}));
    EXPECT_THAT(owners_target_grid, Eq(std::array{expected_owners_target_grid}));
}

TEST(GridLayout, CalculatesTheOverlayOfTwoGridsIn2D) {
    constexpr auto num_processors_x = 2;
    constexpr auto num_processors_y = num_processors_x;

    const auto processor_grid = ProcessorGrid<2>{{num_processors_y, num_processors_x}};

    const auto x_origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto &y_origin_blocks = x_origin_blocks;

    const auto grid = GridLayout(std::array{x_origin_blocks, y_origin_blocks});

    const auto x_target_blocks =
            std::vector{{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}}};
    const auto &y_target_blocks = x_target_blocks;

    const auto target_grid = GridLayout(std::array{x_target_blocks, y_target_blocks});

    const auto overlay = grid.get_overlay(target_grid, processor_grid);
    const auto &grid_overlay = overlay.get_grid();
    const auto &owners_target_grid = overlay.get_owners_target_grid();

    const auto x_expected_blocks_overlay =
            std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 0}, Block{{2, 4}, 1}};
    const auto &y_expected_blocks_overlay = x_expected_blocks_overlay;

    const auto x_expected_owners_target_grid = std::vector{std::vector{0, 1, 1}};
    const auto &y_expected_owners_target_grid = x_expected_owners_target_grid;

    EXPECT_THAT(grid_overlay.get_blocks(),
                Eq(std::array{x_expected_blocks_overlay, y_expected_blocks_overlay}));
    EXPECT_THAT(owners_target_grid,
                Eq(std::array{x_expected_owners_target_grid, y_expected_owners_target_grid}));
}

TEST(GridLayout, OverlayCalculationProducesSameIntervalsButDifferentOwnersIfGridsExchanged) {
    const auto processor_grid = ProcessorGrid<1>{{2}};
    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto grid = GridLayout(std::array{origin_blocks});

    const auto target_blocks = std::vector{{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    const auto overlay = target_grid.get_overlay(grid, processor_grid);
    const auto &grid_overlay = overlay.get_grid();
    const auto &owners_target_grid = overlay.get_owners_target_grid();

    const auto expected_blocks_overlay =
            std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}};
    const auto expected_owners_target_grid = std::vector{std::vector{0, 0, 1}};


    EXPECT_THAT(grid_overlay.get_blocks(), Eq(std::array{expected_blocks_overlay}));
    EXPECT_THAT(owners_target_grid, Eq(std::array{expected_owners_target_grid}));
}

TEST(GridLayout, OverlayNeedsGridToHaveSameGlobalIntervalStartAndEnd) {
    const auto processor_grid = ProcessorGrid<1>{{2}};
    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto grid = GridLayout(std::array{origin_blocks});

    const auto target_blocks = std::vector{{Block{{1, 2}, 1}, Block{{2, 4}, 1}}};
    const auto grid_different_block_start = GridLayout(std::array{target_blocks});

    const auto target_blocks_diff_end =
            std::vector{{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 5}, 1}}};
    const auto grid_different_block_end = GridLayout(std::array{target_blocks_diff_end});


    EXPECT_THROW(const auto _ = grid.get_overlay(grid_different_block_start, processor_grid),
                 std::invalid_argument);
    EXPECT_THROW(const auto _ = grid.get_overlay(grid_different_block_end, processor_grid),
                 std::invalid_argument);
}

TEST(GridLayout, OverlayWorksFromManyToOne) {
    const auto processor_grid = ProcessorGrid<1>{{2}};
    const auto origin_blocks = std::vector{{Block{{0, 4}, 0}, Block{{4, 8}, 1}, Block{{8, 12}, 1}}};
    const auto grid = GridLayout(std::array{origin_blocks});

    const auto target_blocks = std::vector{{Block{{0, 12}, 0}}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    const auto overlay = grid.get_overlay(target_grid, processor_grid);
    const auto &grid_overlay = overlay.get_grid();
    const auto &owners_target_grid = overlay.get_owners_target_grid();

    EXPECT_THAT(grid_overlay.get_blocks(), Eq(grid.get_blocks()));
    EXPECT_THAT(owners_target_grid[0], Eq(std::vector{0, 0, 0}));
}

TEST(GridLayout, OverlayWorksFroOneToMany) {
    const auto origin_blocks = std::vector{Block{{0, 12}, 0}};
    const auto grid = GridLayout(std::array{origin_blocks});

    const auto target_processor_grid = ProcessorGrid<1>{{2}};
    const auto target_blocks = std::vector{Block{{0, 6}, 0}, Block{{6, 12}, 1}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    const auto overlay = grid.get_overlay(target_grid, target_processor_grid);
    const auto &grid_overlay = overlay.get_grid();
    const auto &owners_target_grid = overlay.get_owners_target_grid();

    const auto expected_blocks = std::vector{Block{{0, 6}, 0}, Block{{6, 12}, 0}};

    EXPECT_THAT(grid_overlay.get_blocks(), Eq(std::array{expected_blocks}));
    EXPECT_THAT(owners_target_grid[0], Eq(std::vector{0, 1}));
}

TEST(GridLayout, CanReturnTheLocalGridOfAProcessor) {
    const auto processor_grid = ProcessorGrid<1>{{2}};
    const auto blocks = std::vector{Block{{0, 1}, 0}, Block{{1, 4}, 1}, Block{{4, 6}, 0}};
    const auto grid_layout = GridLayout(std::array{blocks});

    const auto expected_blocks_local_grid_0 = std::vector{Block{{0, 1}, 0}, Block{{1, 3}, 0}};
    const auto expected_blocks_local_grid_1 = std::vector{Block{{0, 3}, 1}};

    EXPECT_THAT(grid_layout.get_local_grid(0, processor_grid).get_blocks()[0],
                Eq(expected_blocks_local_grid_0));
    EXPECT_THAT(grid_layout.get_local_grid(1, processor_grid).get_blocks()[0],
                Eq(expected_blocks_local_grid_1));
}

TEST(GridLayout, CanReturnTheLocalGridOfAProcessorIn2D) {
    constexpr auto num_processors_x = 2;
    constexpr auto num_processors_y = 2;

    const auto processor_grid = ProcessorGrid<2>{{num_processors_y, num_processors_x}};
    const auto x_blocks = get_blocks(num_processors_x);
    const auto y_blocks = get_blocks(num_processors_y);

    const auto grid_layout = GridLayout(std::array{x_blocks, y_blocks});

    const auto expected_blocks_local_grid_0 =
            std::array{std::vector{Block{{0, 1}, 0}}, std::vector{Block{{0, 1}, 0}}};
    const auto expected_blocks_local_grid_1 =
            std::array{std::vector{Block{{0, 1}, 0}}, std::vector{Block{{0, 1}, 1}}};
    const auto expected_blocks_local_grid_2 =
            std::array{std::vector{Block{{0, 1}, 1}}, std::vector{Block{{0, 1}, 0}}};
    const auto expected_blocks_local_grid_3 =
            std::array{std::vector{Block{{0, 1}, 1}}, std::vector{Block{{0, 1}, 1}}};


    EXPECT_THAT(grid_layout.get_local_grid(0, processor_grid).get_blocks(),
                Eq(expected_blocks_local_grid_0));
    EXPECT_THAT(grid_layout.get_local_grid(1, processor_grid).get_blocks(),
                Eq(expected_blocks_local_grid_1));
    EXPECT_THAT(grid_layout.get_local_grid(2, processor_grid).get_blocks(),
                Eq(expected_blocks_local_grid_2));
    EXPECT_THAT(grid_layout.get_local_grid(3, processor_grid).get_blocks(),
                Eq(expected_blocks_local_grid_3));
}

auto get_blocks(const int num_blocks) -> std::vector<Block> {
    std::vector<Block> blocks{};
    blocks.reserve(num_blocks);
    for (int i = 0; i < num_blocks; ++i) { blocks.push_back(Block{{i, i + 1}, i}); }
    return blocks;
}
