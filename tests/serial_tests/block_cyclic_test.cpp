#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block_cyclic.hpp>

using testing::Eq;

TEST(BlockCyclic, CreatesBlocksOfGivenSize) {
    constexpr int block_size = 2;
    constexpr int num_values = 6;
    constexpr int num_procs = 1;
    const auto data_distribution = reshuffle::BlockCyclic(block_size, num_values, num_procs);

    const auto blocks = data_distribution.get_blocks();

    EXPECT_THAT(blocks, Eq(std::vector{reshuffle::Block{0, 2}, reshuffle::Block{2, 4},
                                       reshuffle::Block{4, 6}}));
}

TEST(BlockCyclic, MakesLastBlockSmallerIfNumValuesNoDivisible) {
    constexpr int block_size = 2;
    constexpr int num_values = 5;
    constexpr int num_procs = 1;
    const auto data_distribution = reshuffle::BlockCyclic(block_size, num_values, num_procs);

    const auto blocks = data_distribution.get_blocks();

    EXPECT_THAT(blocks, Eq(std::vector{reshuffle::Block{0, 2}, reshuffle::Block{2, 4},
                                       reshuffle::Block{4, 5}}));
}

TEST(BlockCyclic, AssignsBlocksInRoundRobbinFashion) {
    constexpr int block_size = 2;
    constexpr int num_values = 6;
    constexpr int num_procs = 2;
    const auto data_distribution = reshuffle::BlockCyclic(block_size, num_values, num_procs);

    auto result = std::vector<reshuffle::rank_id>{};

    for (int i = 0; i < num_values; ++i) { result.push_back(data_distribution.get_rank_id(i)); }

    EXPECT_THAT(result, Eq(std::vector<reshuffle::rank_id>{0, 0, 1, 1, 0, 0}));
}

TEST(BlockCyclic, CalculatesTheNumberOfValuesPerProcessor) {
    constexpr int block_size = 2;
    constexpr int num_values = 7;
    constexpr int num_procs = 3;
    const auto data_distribution = reshuffle::BlockCyclic(block_size, num_values, num_procs);

    EXPECT_THAT(data_distribution.get_num_values(0), Eq(3));
    EXPECT_THAT(data_distribution.get_num_values(1), Eq(2));
    EXPECT_THAT(data_distribution.get_num_values(2), Eq(2));
}

TEST(MakeBlockWise, CanBeUsedToGetABlockWiseFromBlockCyclic) {
    constexpr int num_values = 10;
    constexpr int num_blocks = 2;
    const auto data_distribution = reshuffle::make_block_wise(num_values, num_blocks);

    const auto blocks = data_distribution.get_blocks();

    EXPECT_THAT(blocks, Eq(std::vector{reshuffle::Block{0, 5}, reshuffle::Block{5, 10}}));
}

TEST(MakeBlockWise, MakesLastBlocksSmallerIfNotDivisible) {
    constexpr int num_values = 11;
    constexpr int num_blocks = 2;
    const auto data_distribution = reshuffle::make_block_wise(num_values, num_blocks);

    const auto blocks = data_distribution.get_blocks();

    EXPECT_THAT(blocks, Eq(std::vector{reshuffle::Block{0, 6}, reshuffle::Block{6, 11}}));
}

TEST(MakeBlockWise, ThrowsIfZeroNumBlocksPassed) {
    constexpr int num_values = 11;
    EXPECT_THROW(const auto data_distribution = reshuffle::make_block_wise(num_values, 0),
                 std::invalid_argument);
}