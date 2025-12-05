#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <dimensions.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::Eq;

TEST(Dimension, CanBeCreatedFromAListOfIntegers) {
    constexpr auto dimension = Dimensions<2>{1, 2};

    EXPECT_THAT(dimension[0], Eq(1));
    EXPECT_THAT(dimension[1], Eq(2));
}

TEST(Dimension, CanCalculateTheTotalNumberOfValues) {
    constexpr auto dimension = Dimensions<2>{2, 2};

    EXPECT_THAT(calc_total_num_values(dimension), Eq(4));
}

TEST(Dimension, CanBeAdded) {
    constexpr auto dimension1 = Dimensions<2>{1, 2};
    constexpr auto dimension2 = Dimensions<2>{2, 3};

    constexpr auto expected = Dimensions<2>{3, 5};
    EXPECT_THAT(dimension1 + dimension2, Eq(expected));
}


TEST(GetRank, ReturnsTheNumberOfDimensionsOfANestedContainer) {
    const auto one_dim = std::vector{1, 2, 3};
    const auto two_dim = std::vector{one_dim, one_dim};
    const auto three_dim = std::vector{two_dim, two_dim};

    EXPECT_THAT(get_rank(one_dim), Eq(1));
    EXPECT_THAT(get_rank(two_dim), Eq(2));
    EXPECT_THAT(get_rank(three_dim), Eq(3));
}

TEST(GetRank, WorksWithEmptyContainers) {
    EXPECT_THAT(get_rank(std::vector<int>{}), Eq(1));
    EXPECT_THAT(get_rank(std::vector<std::vector<int>>{}), Eq(2));
    EXPECT_THAT(get_rank(std::vector<std::vector<std::vector<int>>>{}), Eq(3));
}

TEST(GetDimensions, ReturnsTheDimensionsOfANestedContainer) {
    const auto one_dim = std::vector{1, 2, 3};
    const auto two_dim = std::vector{one_dim, one_dim};
    const auto three_dim = std::vector{two_dim, two_dim};

    EXPECT_THAT(get_dimensions(one_dim), Eq(std::array{3}));
    EXPECT_THAT(get_dimensions(two_dim), Eq(std::array{2, 3}));
    EXPECT_THAT(get_dimensions(three_dim), Eq(std::array{2, 2, 3}));
}
