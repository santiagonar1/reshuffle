#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <data_distribution.hpp>

using ::testing::Eq;

TEST(BlockWise, SplitsADomainInEqualBlocksIfEvenlyDivisible) {
    const auto block_data_distribution = reshuffle::BlockWise(2);
    const auto subdomains = block_data_distribution.get_blocks(10);

    EXPECT_THAT(subdomains, Eq(std::vector{reshuffle::Block{0, 5}, reshuffle::Block{5, 10}}));
}

TEST(BlockWise, AddsRemainingElementsToLastBlock) {
    const auto block_data_distribution = reshuffle::BlockWise(2);
    const auto subdomains = block_data_distribution.get_blocks(11);

    EXPECT_THAT(subdomains, Eq(std::vector{reshuffle::Block{0, 5}, reshuffle::Block{5, 11}}));
}

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