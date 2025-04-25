#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <dimensions.hpp>

using namespace reshuffle;

using testing::Eq;

TEST(Dimension, CanBeCreatedFromAListOfIntegers) {
    constexpr auto dimension = Dimensions{1, 2};

    EXPECT_THAT(dimension[0], Eq(1));
    EXPECT_THAT(dimension[1], Eq(2));
}

TEST(Dimension, CanCalculateTheTotalNumberOfValues) {
    constexpr auto dimension = Dimensions{2, 2};

    EXPECT_THAT(calc_total_num_values(dimension), Eq(4));
}
