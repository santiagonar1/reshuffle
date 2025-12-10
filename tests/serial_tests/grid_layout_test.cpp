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

    const auto grid_layout = GridLayout(std::array{y_blocks, x_blocks});

    EXPECT_THAT(grid_layout.get_blocks(), Eq(std::array{y_blocks, x_blocks}));
}

TEST(GridLayout, CanReturnTheOwnerOfABlock) {
    constexpr auto num_processors = 2;

    const auto processor_grid = ProcessorGrid{num_processors};
    const auto blocks = get_blocks(num_processors);
    const auto grid_layout = GridLayout(std::array{blocks});

    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<1>{0}, processor_grid), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<1>{1}, processor_grid), Eq(1));
}

TEST(GridLayout, CanReturnTheOwnerOfABlockIn2DHorizontalPartition) {
    constexpr auto num_processors_x = 2;
    constexpr auto num_processors_y = 1;

    const auto processor_grid = ProcessorGrid{num_processors_y, num_processors_x};
    const auto x_blocks = get_blocks(num_processors_x);
    const auto y_blocks = get_blocks(num_processors_y);

    const auto grid_layout = GridLayout(std::array{y_blocks, x_blocks});

    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 0}, processor_grid), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 1}, processor_grid), Eq(1));
}

TEST(GridLayout, CanReturnTheOwnerOfABlockIn2DVerticalPartition) {
    constexpr auto num_processors_x = 1;
    constexpr auto num_processors_y = 2;

    const auto processor_grid = ProcessorGrid{num_processors_y, num_processors_x};
    const auto x_blocks = get_blocks(num_processors_x);
    const auto y_blocks = get_blocks(num_processors_y);

    const auto grid_layout = GridLayout(std::array{y_blocks, x_blocks});

    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 0}, processor_grid), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{1, 0}, processor_grid), Eq(1));
}

TEST(GridLayout, CanReturnTheOwnerOfABlockIn2DCrossPartition) {
    constexpr auto num_processors_x = 2;
    constexpr auto num_processors_y = 2;

    const auto processor_grid = ProcessorGrid{num_processors_y, num_processors_x};
    const auto x_blocks = get_blocks(num_processors_x);
    const auto y_blocks = get_blocks(num_processors_y);

    const auto grid_layout = GridLayout(std::array{y_blocks, x_blocks});

    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 0}, processor_grid), Eq(0));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{0, 1}, processor_grid), Eq(1));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{1, 0}, processor_grid), Eq(2));
    EXPECT_THAT(grid_layout.get_block_owner(Coordinates<2>{1, 1}, processor_grid), Eq(3));
}

TEST(GridLayout, ThrowsWhenAskedForABlockIdOutOfRange) {
    constexpr auto num_processors = 2;

    const auto processor_grid = ProcessorGrid{num_processors};
    const auto blocks = get_blocks(num_processors);
    const auto grid_layout = GridLayout(std::array{blocks});

    constexpr auto block_id_out_of_range = num_processors;

    EXPECT_THROW(auto _ = grid_layout.get_block_owner(Coordinates<1>{block_id_out_of_range},
                                                      processor_grid),
                 std::out_of_range);
}

TEST(GridLayout, CanReturnTheLocalGridOfAProcessor) {
    const auto processor_grid = ProcessorGrid{2};
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

    const auto processor_grid = ProcessorGrid{num_processors_y, num_processors_x};
    const auto x_blocks = get_blocks(num_processors_x);
    const auto y_blocks = get_blocks(num_processors_y);

    const auto grid_layout = GridLayout(std::array{y_blocks, x_blocks});

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

TEST(GridLayout, CanBeCompared) {
    const auto blocks = std::vector{Block{{0, 1}, 0}, Block{{1, 4}, 1}, Block{{4, 6}, 0}};
    const auto different_blocks = std::vector{Block{{0, 6}, 0}};
    const auto grid_layout = GridLayout(std::array{blocks});
    const auto same_grid_layout = GridLayout(std::array{blocks});
    const auto different_grid_layout = GridLayout(std::array{different_blocks});

    EXPECT_TRUE(grid_layout == same_grid_layout);
    EXPECT_FALSE(grid_layout == different_grid_layout);
}

auto get_blocks(const int num_blocks) -> std::vector<Block> {
    std::vector<Block> blocks{};
    blocks.reserve(num_blocks);
    for (int i = 0; i < num_blocks; ++i) { blocks.push_back(Block{{i, i + 1}, i}); }
    return blocks;
}
