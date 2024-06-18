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
