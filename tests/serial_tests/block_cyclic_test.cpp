#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block_cyclic.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::Eq;
using testing::Lt;

template<std::size_t N>
auto make_block_wise_distribution(const Dimensions<N> &num_global_values,
                                  const ProcessorGrid<N> &processor_grid) -> BlockCyclic<N>;

TEST(BlockCyclic, CreatesGridWithBlocksOfGivenSize) {
    constexpr int block_size = 2;
    constexpr int num_values = 6;
    const auto processor_grid = ProcessorGrid{1};
    const auto data_distribution = BlockCyclic({num_values}, {block_size}, processor_grid);

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto expected = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 0}, Block{{4, 6}, 0}};

    EXPECT_THAT(blocks, Eq(expected));
}

TEST(BlockCyclic, MakesLastBlockSmallerIfNumValuesNoDivisible) {
    constexpr int block_size = 2;
    constexpr int num_values = 5;
    constexpr int num_ranks = 1;
    const auto processor_grid = ProcessorGrid{num_ranks};
    const auto data_distribution = BlockCyclic({num_values}, {block_size}, processor_grid);

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto expected = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 0}, Block{{4, 5}, 0}};

    EXPECT_THAT(blocks, Eq(expected));
}

TEST(BlockCyclic, CalculatesTheNumberOfBlocksPerDimension) {
    constexpr int block_size = 2;
    constexpr int num_values = 5;
    constexpr int num_ranks = 1;

    const auto processor_grid = ProcessorGrid{num_ranks};
    const auto data_distribution = BlockCyclic({num_values}, {block_size}, processor_grid);

    EXPECT_THAT(data_distribution.get_num_blocks_per_dimension(), Eq(Dimensions{3}));
}

TEST(BlockCyclic, AssignsBlocksInRoundRobbinFashion) {
    constexpr int block_size = 2;
    constexpr int num_values = 6;
    constexpr int num_ranks = 2;
    const auto processor_grid = ProcessorGrid{num_ranks};
    const auto data_distribution = BlockCyclic({num_values}, {block_size}, processor_grid);

    auto result = std::vector<reshuffle::RankId>{};
    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    for (int i = 0; i < blocks.size(); ++i) {
        const auto block_coordinates = Coordinates<1>{i};
        result.push_back(data_distribution.get_grid_layout().get_block_owner(block_coordinates,
                                                                             processor_grid));
    }


    const auto expected = std::vector{0, 1, 0};
    EXPECT_THAT(result, Eq(expected));
}

TEST(BlockCylic, CanBeCompared) {
    constexpr auto num_values = 1000;
    constexpr auto num_ranks = 10;
    const auto processor_grid = ProcessorGrid{num_ranks};
    const auto distribution = make_block_wise_distribution({num_values}, processor_grid);
    const auto different_distribution = BlockCyclic{{num_values}, {10}, processor_grid};
    const auto same_distribution = make_block_wise_distribution({num_values}, processor_grid);

    EXPECT_TRUE(distribution != different_distribution);
    EXPECT_TRUE(distribution == same_distribution);
}

TEST(BlockCyclic, InMultipleDimensionsIsEquivalentToSeveralInOneDimension) {
    constexpr auto x_block_size = 4;
    constexpr auto y_block_size = 2;
    constexpr auto z_block_size = 3;

    constexpr auto x_num_values = 10;
    constexpr auto y_num_values = 15;
    constexpr auto z_num_values = 20;

    constexpr auto x_num_ranks = 2;
    constexpr auto y_num_ranks = 3;
    constexpr auto z_num_ranks = 4;

    const auto x_processor_grid = ProcessorGrid{x_num_ranks};
    const auto y_processor_grid = ProcessorGrid{y_num_ranks};
    const auto z_processor_grid = ProcessorGrid{z_num_ranks};

    const auto x_data_distribution = BlockCyclic({x_num_values}, {x_block_size}, x_processor_grid);
    const auto y_data_distribution = BlockCyclic({y_num_values}, {y_block_size}, y_processor_grid);
    const auto z_data_distribution = BlockCyclic({z_num_values}, {z_block_size}, z_processor_grid);

    const auto x_blocks = x_data_distribution.get_grid_layout().get_blocks().at(0);
    const auto y_blocks = y_data_distribution.get_grid_layout().get_blocks().at(0);
    const auto z_blocks = z_data_distribution.get_grid_layout().get_blocks().at(0);

    const auto processor_grid = ProcessorGrid{x_num_ranks, y_num_ranks, z_num_ranks};
    const auto data_distribution =
            BlockCyclic({x_num_values, y_num_values, z_num_values},
                        {x_block_size, y_block_size, z_block_size}, processor_grid);

    EXPECT_THAT(data_distribution.get_grid_layout().get_blocks()[0], Eq(x_blocks));
    EXPECT_THAT(data_distribution.get_grid_layout().get_blocks()[1], Eq(y_blocks));
    EXPECT_THAT(data_distribution.get_grid_layout().get_blocks()[2], Eq(z_blocks));
}

TEST(BlockCyclic, CanBeCloned) {
    constexpr auto num_values = 100;
    constexpr auto num_ranks = 10;
    const auto processor_grid = ProcessorGrid{num_ranks};
    constexpr auto dimensions = Dimensions{num_values};
    const auto distribution = BlockCyclic{dimensions, {10}, processor_grid};

    const auto clone = distribution.clone();
    EXPECT_THAT(*clone, Eq(distribution));
}

TEST(IsBlockWise, ReturnsTrueIfDistributionIsBlockWise) {
    constexpr int num_values = 10;
    constexpr int num_ranks = 3;
    const auto processor_grid = ProcessorGrid{num_ranks};
    const auto block_wise = make_block_wise_distribution({num_values}, processor_grid);

    EXPECT_TRUE(block_wise.is_block_wise());
}

TEST(IsBlockWise, ReturnsFlaseIfDistributionIsNotBlockWise) {
    constexpr int block_size = 2;
    constexpr int num_values = 9;
    constexpr int num_ranks = 2;

    const auto processor_grid = ProcessorGrid{num_ranks};
    const auto non_block_wise = BlockCyclic({num_values}, {block_size}, processor_grid);

    EXPECT_FALSE(non_block_wise.is_block_wise());
}

TEST(IsBlockWise, ADistributionWithOnlyOneRankOnEachDimensionIsBlockWise) {
    constexpr int block_size = 2;
    constexpr int num_values = 5;
    constexpr int num_ranks = 1;

    const auto processor_grid = ProcessorGrid{num_ranks};
    const auto one_rank_distribution = BlockCyclic({num_values}, {block_size}, processor_grid);

    EXPECT_TRUE(one_rank_distribution.is_block_wise());
}

TEST(CreateBlocks, CreatesBlocksOfGivenSizeAndAssignsThemRoundRobinToProcessors) {
    constexpr auto num_values = 9;
    constexpr auto block_size = 3;
    constexpr auto num_processors = 2;

    const auto expected = std::vector{Block{{0, 3}, 0}, Block{{3, 6}, 1}, Block{{6, 9}, 0}};
    EXPECT_THAT(create_blocks(num_values, block_size, num_processors), Eq(expected));
}

TEST(CreateBlocks, MakesLastBlockSmallerIfNoDivisible) {
    constexpr auto num_values = 8;
    constexpr auto block_size = 3;
    constexpr auto num_processors = 2;

    const auto expected = std::vector{Block{{0, 3}, 0}, Block{{3, 6}, 1}, Block{{6, 8}, 0}};
    EXPECT_THAT(create_blocks(num_values, block_size, num_processors), Eq(expected));
}

TEST(CreateBlocks, CanBeUsedInMultipleDimensions) {
    constexpr auto num_values_x = 9;
    constexpr auto num_values_y = 8;
    constexpr auto block_size_x = 3;
    constexpr auto block_size_y = 3;
    constexpr auto num_processors = 2;

    const auto num_values = Dimensions{num_values_y, num_values_x};
    const auto block_size = Dimensions{block_size_y, block_size_x};
    const auto processor_grid = ProcessorGrid{num_processors, num_processors};

    const auto expected_x = std::vector{Block{{0, 3}, 0}, Block{{3, 6}, 1}, Block{{6, 9}, 0}};
    const auto expected_y = std::vector{Block{{0, 3}, 0}, Block{{3, 6}, 1}, Block{{6, 8}, 0}};

    EXPECT_THAT(create_blocks(num_values, block_size, processor_grid),
                Eq(std::array{expected_y, expected_x}));
}

template<std::size_t N>
auto make_block_wise_distribution(const Dimensions<N> &num_global_values,
                                  const ProcessorGrid<N> &processor_grid) -> BlockCyclic<N> {
    auto block_sizes = Dimensions<N>{};
    for (int i = 0; i < N; ++i) {
        const auto num_processors = processor_grid.get_dimensions()[i];
        block_sizes[i] = std::ceil(static_cast<double>(num_global_values[i]) / num_processors);
    }
    return BlockCyclic{num_global_values, block_sizes, processor_grid};
}