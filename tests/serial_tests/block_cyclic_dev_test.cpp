#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block_cyclic.hpp>

using namespace reshuffle::dev;
using namespace reshuffle::internal;

using testing::Eq;
using testing::Lt;

TEST(BlockCyclic, CreatesGridWithBlocksOfGivenSize) {
    constexpr int block_size = 2;
    constexpr int num_values = 6;
    const auto processor_grid = ProcessorGrid<1>{{1}};
    const auto data_distribution = BlockCyclic(num_values, block_size, processor_grid);

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto expected = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 0}, Block{{4, 6}, 0}};

    EXPECT_THAT(blocks, Eq(expected));
}

TEST(BlockCyclic, MakesLastBlockSmallerIfNumValuesNoDivisible) {
    constexpr int block_size = 2;
    constexpr int num_values = 5;
    constexpr int num_ranks = 1;
    const auto processor_grid = ProcessorGrid<1>{{num_ranks}};
    const auto data_distribution = BlockCyclic(num_values, block_size, processor_grid);

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto expected = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 0}, Block{{4, 5}, 0}};

    EXPECT_THAT(blocks, Eq(expected));
}

TEST(BlockCyclic, AssignsBlocksInRoundRobbinFashion) {
    constexpr int block_size = 2;
    constexpr int num_values = 6;
    constexpr int num_ranks = 2;
    const auto processor_grid = ProcessorGrid<1>{{num_ranks}};
    const auto data_distribution = BlockCyclic(num_values, block_size, processor_grid);

    auto result = std::vector<reshuffle::rank_id>{};
    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    for (int i = 0; i < blocks.size(); ++i) {
        const auto block_coordinates = Coordinates{i};
        result.push_back(data_distribution.get_grid_layout().get_block_owner(block_coordinates,
                                                                             processor_grid));
    }


    const auto expected = std::vector{0, 1, 0};
    EXPECT_THAT(result, Eq(expected));
}

TEST(BlockCylic, CanBeCompared) {
    constexpr auto num_values = 1000;
    constexpr auto num_ranks = 10;
    const auto processor_grid = ProcessorGrid<1>{{num_ranks}};
    const auto distribution = make_block_wise_distribution(num_values, processor_grid);
    const auto different_distribution = BlockCyclic{num_values, 10, processor_grid};
    const auto same_distribution = make_block_wise_distribution(num_values, processor_grid);

    EXPECT_TRUE(distribution != different_distribution);
    EXPECT_TRUE(distribution == same_distribution);
}

TEST(MakeBlockWise, CanBeUsedToGetABlockWiseFromBlockCyclic) {
    constexpr int num_values = 10;
    constexpr int num_ranks = 2;
    const auto processor_grid = ProcessorGrid<1>{{num_ranks}};
    const auto data_distribution = make_block_wise_distribution(num_values, processor_grid);

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto expected = std::vector{Block{{0, 5}, 0}, Block{{5, 10}, 1}};

    EXPECT_THAT(blocks, Eq(expected));
}

TEST(MakeBlockWise, AssignsOneBlockPerRank) {
    constexpr int num_values = 10;
    constexpr int num_ranks = 3;
    const auto processor_grid = ProcessorGrid<1>{{num_ranks}};
    const auto data_distribution = make_block_wise_distribution(num_values, processor_grid);

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto num_blocks = static_cast<int>(blocks.size());

    EXPECT_THAT(num_blocks, Eq(num_ranks));
}

TEST(MakeBlockWise, MakesLastBlocksSmallerIfNotDivisible) {
    constexpr int num_values = 11;
    constexpr int num_ranks = 2;
    const auto processor_grid = ProcessorGrid<1>{{num_ranks}};
    const auto data_distribution = make_block_wise_distribution(num_values, processor_grid);

    const auto blocks = data_distribution.get_grid_layout().get_blocks().at(0);
    const auto size_first_block = blocks.at(0).get_interval().get_length();
    const auto size_last_block = blocks.at(1).get_interval().get_length();

    EXPECT_THAT(size_last_block, Lt(size_first_block));
}