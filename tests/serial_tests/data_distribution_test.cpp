#ifndef RESHUFFLE_DATA_DISTRIBUTION_TEST_HPP
#define RESHUFFLE_DATA_DISTRIBUTION_TEST_HPP

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <data_distribution.hpp>

using ::testing::Eq;

TEST(BlockWise, SplitsADomainInEqualBlocksIfEvenlyDivisible) {
    const auto block_data_distribution = reshuffle::BlockWise(2);
    const auto subdomains = block_data_distribution.get_subdomains(10);

    EXPECT_THAT(subdomains,
                Eq(std::vector{reshuffle::Subdomain{0, 5}, reshuffle::Subdomain{5, 10}}));
}

#endif//RESHUFFLE_DATA_DISTRIBUTION_TEST_HPP
