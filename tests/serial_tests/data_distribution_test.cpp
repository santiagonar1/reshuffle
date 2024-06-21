#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <data_distribution.hpp>

using ::testing::Eq;

TEST(BlockCyclic, CreatesBlocksOfGivenSize) {
    const auto data_distribution = reshuffle::BlockCyclic(2, 6);
    const auto blocks = data_distribution.get_blocks();

    EXPECT_THAT(blocks, Eq(std::vector{reshuffle::Block{0, 2}, reshuffle::Block{2, 4},
                                       reshuffle::Block{4, 6}}));
}

TEST(BlockCyclic, MakesLastBlockSmallerIfNumValuesNoDivisible) {
    const auto data_distribution = reshuffle::BlockCyclic(2, 5);
    const auto blocks = data_distribution.get_blocks();

    EXPECT_THAT(blocks, Eq(std::vector{reshuffle::Block{0, 2}, reshuffle::Block{2, 4},
                                       reshuffle::Block{4, 5}}));
}

TEST(MakeBlockWise, CanBeUsedToGetABlockWiseFromBlockCyclic) {
    const auto data_distribution = reshuffle::make_block_wise(10, 2);
    const auto blocks = data_distribution.get_blocks();

    EXPECT_THAT(blocks, Eq(std::vector{reshuffle::Block{0, 5}, reshuffle::Block{5, 10}}));
}

TEST(MakeBlockWise, MakesLastBlocksSmallerIfNotDivisible) {
    const auto data_distribution = reshuffle::make_block_wise(11, 2);
    const auto blocks = data_distribution.get_blocks();

    EXPECT_THAT(blocks, Eq(std::vector{reshuffle::Block{0, 6}, reshuffle::Block{6, 11}}));
}