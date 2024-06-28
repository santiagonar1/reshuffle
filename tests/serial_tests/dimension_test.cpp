#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <dimensions.hpp>

using ::testing::Eq;

TEST(Dimension, CanBeCreatedFromAListOfIntegers) {
    const auto dimension = reshuffle::Dimension<2>({1, 2});

    EXPECT_THAT(dimension.get_num_values_dim(0), Eq(1));
    EXPECT_THAT(dimension.get_num_values_dim(1), Eq(2));
}

TEST(Dimension, ThrowsWhenAccessingDimensionOutOfBounds) {
    const auto dimension = reshuffle::Dimension<2>({1, 2});

    EXPECT_THROW(int dummy = dimension.get_num_values_dim(3), std::out_of_range);
}

TEST(Dimension, CanCalculateTheTotalNumberOfValues) {
    const auto dimension = reshuffle::Dimension<2>({2, 2});

    EXPECT_THAT(dimension.get_total_number_of_values(), Eq(4));
}
