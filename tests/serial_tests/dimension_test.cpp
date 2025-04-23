#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <dimensions.hpp>

using testing::Eq;

TEST(Dimension, CanBeCreatedFromAListOfIntegers) {
    constexpr auto dimension = reshuffle::internal::Dimensions<2>({1, 2});

    EXPECT_THAT(dimension[0], Eq(1));
    EXPECT_THAT(dimension[1], Eq(2));
}

TEST(Dimension, CanCalculateTheTotalNumberOfValues) {
    constexpr auto dimension = reshuffle::internal::Dimensions<2>({2, 2});

    EXPECT_THAT(reshuffle::internal::calc_total_num_values(dimension), Eq(4));
}
