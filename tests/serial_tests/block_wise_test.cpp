#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block_wise.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;
using namespace reshuffle::distribution;
using namespace reshuffle::distribution::internal;

using testing::Eq;
using testing::Lt;

TEST(BlockWise, AssignsOneBlockPerProcessor) {
    constexpr int num_values = 9;
    constexpr int num_processors = 3;
    const auto processor_grid = ProcessorGrid{num_processors};
    const auto data_distribution = BlockWise{{num_values}, processor_grid};

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto expected = std::vector{Block{{0, 3}, 0}, Block{{3, 6}, 1}, Block{{6, 9}, 2}};

    EXPECT_THAT(blocks, Eq(expected));
}

TEST(BlockWise, MakesLastBlockSmallerIfNotDivisible) {
    constexpr int num_values = 10;
    constexpr int num_ranks = 3;
    const auto processor_grid = ProcessorGrid{num_ranks};
    const auto data_distribution = BlockWise{{num_values}, processor_grid};

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto expected = std::vector{Block{{0, 4}, 0}, Block{{4, 8}, 1}, Block{{8, 10}, 2}};

    EXPECT_THAT(blocks, Eq(expected));
}

TEST(BlockWise, BlocksPerDimensionIsEqualToNumProcessorsPerDimension) {
    constexpr int num_values = 9;
    constexpr int num_processors = 3;
    const auto processor_grid = ProcessorGrid{num_processors};
    const auto data_distribution = BlockWise{{num_values}, processor_grid};

    EXPECT_THAT(data_distribution.get_num_blocks_per_dimension(),
                Eq(processor_grid.get_dimensions()));
}

TEST(BlockWise, CanBeCloned) {
    constexpr auto num_values = 100;
    constexpr auto num_ranks = 10;
    const auto processor_grid = ProcessorGrid{num_ranks};
    constexpr auto dimensions = Dimensions{num_values};
    const auto distribution = BlockWise{dimensions, processor_grid};

    const auto clone = distribution.clone();
    EXPECT_THAT(*clone, Eq(distribution));
}

TEST(CreateEvenlyBlocks, CreatesOneBlockPerProcessor) {
    constexpr auto num_values = 10;
    constexpr auto num_processors = 2;

    const auto expected = std::vector{Block{{0, 5}, 0}, Block{{5, 10}, 1}};
    EXPECT_THAT(create_evenly_blocks(num_values, num_processors), Eq(expected));
}

TEST(CreateEvenlyBlocks, MakesLastBlockSmallerIfNotDivisible) {
    constexpr auto num_values = 11;
    constexpr auto num_processors = 2;

    const auto expected = std::vector{Block{{0, 6}, 0}, Block{{6, 11}, 1}};
    EXPECT_THAT(create_evenly_blocks(num_values, num_processors), Eq(expected));
}

TEST(CreateEvenlyBlocks, CanBeUsedInMultipleDimensions) {
    constexpr auto num_values_x = 10;
    constexpr auto num_values_y = 11;
    constexpr auto num_processors_x = 2;
    constexpr auto num_processors_y = 2;

    const auto num_values = Dimensions{num_values_y, num_values_x};
    const auto processor_grid = ProcessorGrid{num_processors_y, num_processors_x};

    const auto expected_x = std::vector{Block{{0, 5}, 0}, Block{{5, 10}, 1}};
    const auto expected_y = std::vector{Block{{0, 6}, 0}, Block{{6, 11}, 1}};

    EXPECT_THAT(create_evenly_blocks(num_values, processor_grid),
                Eq(std::array{expected_y, expected_x}));
}

TEST(GetAllValuesInRoot, ReturnsBlockWiseDistributionWithOnlyOneRank) {
    constexpr auto global_dimensions = Dimensions{20};
    const auto all_values_in_root = get_all_values_in_root(global_dimensions);

    const auto expected = BlockWise{global_dimensions, ProcessorGrid{1}};
    EXPECT_THAT(all_values_in_root, Eq(expected));
}