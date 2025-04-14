#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <grid_layout.hpp>

using namespace reshuffle::dev;

using testing::Eq;

TEST(GridLayout, StoresBlocks) {
    const auto blocks = std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 1}};
    const auto grid_layout = GridLayout(blocks);

    EXPECT_THAT(grid_layout.get_blocks(), Eq(blocks));
}

TEST(GridLayout, CanReturnTheOwnerOfABlock) {
    const auto blocks = std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 1}};
    const auto grid_layout = GridLayout(blocks);

    EXPECT_THAT(grid_layout.get_block_owner(0), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(1), Eq(1));
}

TEST(GridLayout, ThrowsWhenAskedForABlockIdOutOfRange) {
    const auto blocks = std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 1}};
    const auto grid_layout = GridLayout(blocks);
    constexpr auto block_id_out_of_range = 2;

    EXPECT_THROW(auto _ = grid_layout.get_block_owner(block_id_out_of_range), std::out_of_range);
}

TEST(GridLayout, CalculatesTheOverlayOfTwoGrids) {
    const auto grid = GridLayout({Block{{0, 2}, 0}, Block{{2, 4}, 1}});
    const auto target_grid = GridLayout({Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}});

    const auto [grid_overlay, owners_target_grid] = grid.get_overlay(target_grid);

    EXPECT_THAT(grid_overlay.get_blocks(),
                Eq(std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 0}, Block{{2, 4}, 1}}));
    EXPECT_THAT(owners_target_grid, Eq(std::vector{0, 1, 1}));
}

TEST(GridLayout, OverlayCalculationProducesSameIntervalsButDifferentOwnersIfGridsExchanged) {
    const auto grid = GridLayout({Block{{0, 2}, 0}, Block{{2, 4}, 1}});
    const auto target_grid = GridLayout({Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}});

    const auto [grid_overlay, owners_grid] = target_grid.get_overlay(grid);

    EXPECT_THAT(grid_overlay.get_blocks(),
                Eq(std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}}));
    EXPECT_THAT(owners_grid, Eq(std::vector{0, 0, 1}));
}

TEST(GridLayout, OverlayNeedsGridToHaveSameGlobalIntervalStartAndEnd) {
    const auto grid = GridLayout({Block{{0, 2}, 0}, Block{{2, 4}, 1}});
    const auto grid_different_block_start = GridLayout({Block{{1, 2}, 1}, Block{{2, 4}, 1}});
    const auto grid_different_block_end =
            GridLayout({Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 5}, 1}});


    EXPECT_THROW(const auto _ = grid.get_overlay(grid_different_block_start),
                 std::invalid_argument);
    EXPECT_THROW(const auto _ = grid.get_overlay(grid_different_block_end), std::invalid_argument);
}

TEST(GridLayout, OverlayWorksFromManyToOne) {
    const auto grid = GridLayout({Block{{0, 4}, 0}, Block{{4, 8}, 1}, Block{{8, 12}, 1}});
    const auto other_grid = GridLayout({Block{{0, 12}, 0}});

    const auto [grid_overlay, owners_target_grid] = grid.get_overlay(other_grid);

    EXPECT_THAT(grid_overlay.get_blocks(), Eq(grid.get_blocks()));
    EXPECT_THAT(owners_target_grid, Eq(std::vector{0, 0, 0}));
}

TEST(GridLayout, CanReturnTheLocalGridOfAProcessor) {
    const auto blocks = std::vector{Block{{0, 1}, 0}, Block{{1, 4}, 1}, Block{{4, 6}, 0}};
    const auto grid_layout = GridLayout(blocks);

    const auto expected_blocks_local_grid_0 = std::vector{Block{{0, 1}, 0}, Block{{1, 3}, 0}};
    const auto expected_blocks_local_grid_1 = std::vector{Block{{0, 3}, 1}};

    EXPECT_THAT(grid_layout.get_local_grid(0).get_blocks(), Eq(expected_blocks_local_grid_0));
    EXPECT_THAT(grid_layout.get_local_grid(1).get_blocks(), Eq(expected_blocks_local_grid_1));
}