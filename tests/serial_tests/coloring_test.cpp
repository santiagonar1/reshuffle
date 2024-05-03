#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <reshuffle.hpp>

using ::testing::Eq;

TEST(CreateColoring, ReturnsColoringPerRankAndGlobalColoring) {
    // i.e., rank 0 has elements 1, 3, 4, and rank 1 elements 0, 2.
    const auto current_coloring = std::vector{1, 0, 1, 0, 0};
    const auto strategy = reshuffle::BlockWise(2);

    const auto [global_coloring, coloring_0] =
            reshuffle::create_coloring(current_coloring, strategy, 0);
    const auto [_, coloring_1] = reshuffle::create_coloring(current_coloring, strategy, 1);

    EXPECT_THAT(coloring_0, Eq(std::vector{0, 1, 1}));
    EXPECT_THAT(coloring_1, Eq(std::vector{0, 1}));
    EXPECT_THAT(global_coloring, Eq(std::vector{0, 0, 1, 1, 1}));
}

TEST(CreateColoring, CanUseBlockWiseStrategyInTwoDimensions) {
    // i.e., 4 values in a 2x2 matrix, previously all in rank 0.
    const auto current_coloring = std::vector<int>(4);
    const auto strategy_x = reshuffle::BlockWise(2);
    const auto strategy_y = reshuffle::BlockWise(2);

    const auto [global_coloring, coloring_0] =
            reshuffle::create_coloring(current_coloring, {2, 2}, {strategy_x, strategy_y}, 0);
    EXPECT_THAT(coloring_0, Eq(std::vector{0, 1, 2, 3}));
}

TEST(CreateColoring, ABlockWiseWithOneBlockIndicatesNoDivision) {
    // i.e., 4 values in a 2x2 matrix, previously all in rank 0.
    const auto current_coloring = std::vector<int>(4);
    const auto strategy_x = reshuffle::BlockWise(1);
    const auto strategy_y = reshuffle::BlockWise(2);

    const auto [global_coloring, coloring_0] =
            reshuffle::create_coloring(current_coloring, {2, 2}, {strategy_x, strategy_y}, 0);
    EXPECT_THAT(coloring_0, Eq(std::vector{0, 0, 1, 1}));
}

TEST(GetSubdomainDimensions, In1DReturnsTheNumberOfValues) {
    constexpr int num_values = 5;
    const auto strategy = reshuffle::BlockWise(2);

    const auto num_values_0 = reshuffle::get_subdomain_dimension(strategy, num_values, 0);
    const auto num_values_1 = reshuffle::get_subdomain_dimension(strategy, num_values, 1);
    const auto num_values_2 = reshuffle::get_subdomain_dimension(strategy, num_values, 2);

    EXPECT_THAT(num_values_0, Eq(2));
    EXPECT_THAT(num_values_1, Eq(3));
    EXPECT_THAT(num_values_2, Eq(0));
}

TEST(GetSubdomainDimensions, In2DReturnsTheSubdomainDimension) {
    const auto strategy_x = reshuffle::BlockWise(2);
    const auto strategy_y = reshuffle::BlockWise(1);

    const auto dimensions_0 =
            reshuffle::get_subdomain_dimension({strategy_x, strategy_y}, {20, 20}, 0);

    EXPECT_THAT(dimensions_0.num_columns, Eq(10));
    EXPECT_THAT(dimensions_0.num_rows, Eq(20));
}
