#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <reshuffle.hpp>

using ::testing::Eq;

TEST(CreateColoring, ReturnsColoringPerRankAndGlobalColoring) {
    // i.e., rank 0 has elements 1, 3, 4, and rank 1 elements 0, 2.
    const auto current_coloring = std::vector{1, 0, 1, 0, 0};
    const auto strategy = reshuffle::BlockWise(2);

    const auto [global_coloring, coloring_0] = reshuffle::create_coloring(current_coloring, strategy, 0);
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

    const auto [global_coloring, coloring_0] = reshuffle::create_coloring(current_coloring, {2, 2},
                                                                          {strategy_x, strategy_y},
                                                                          0);
    EXPECT_THAT(coloring_0, Eq(std::vector{0, 1, 2, 3}));
}

TEST(CreateColoring, ABlockWiseWithOneBlockIndicatesNoDivision) {
    // i.e., 4 values in a 2x2 matrix, previously all in rank 0.
    const auto current_coloring = std::vector<int>(4);
    const auto strategy_x = reshuffle::BlockWise(1);
    const auto strategy_y = reshuffle::BlockWise(2);

    const auto [global_coloring, coloring_0] = reshuffle::create_coloring(current_coloring, {2, 2},
                                                                          {strategy_x, strategy_y},
                                                                          0);
    EXPECT_THAT(coloring_0, Eq(std::vector{0, 0, 1, 1}));
}