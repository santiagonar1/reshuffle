#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <reshuffle.hpp>

using ::testing::Eq;

TEST(GetGlobalAndLocalColoring, WorksIn1D) {
    // i.e., rank 0 has elements 1, 3, 4, and rank 1 elements 0, 2.
    const auto current_coloring = std::vector{1, 0, 1, 0, 0};
    const auto num_values = static_cast<int>(current_coloring.size());
    const auto data_distribution = reshuffle::make_block_wise(num_values, 2);

    const auto [global_coloring, coloring_0] = reshuffle::internal::get_global_and_local_coloring(
            current_coloring, data_distribution, 0);
    const auto [_, coloring_1] = reshuffle::internal::get_global_and_local_coloring(
            current_coloring, data_distribution, 1);

    EXPECT_THAT(coloring_0, Eq(std::vector{0, 1, 1}));
    EXPECT_THAT(coloring_1, Eq(std::vector{0, 0}));
    EXPECT_THAT(global_coloring, Eq(std::vector{0, 0, 0, 1, 1}));
}

TEST(GetGlobalAndLocalColoring, WorksIn2D) {
    // i.e., 4 values in a 2x2 matrix, previously all in rank 0.
    const auto current_coloring = std::vector<int>(4);
    const auto data_distribution_x = reshuffle::make_block_wise(2, 2);
    const auto data_distribution_y = reshuffle::make_block_wise(2, 2);

    const auto data_distributions = std::array{data_distribution_x, data_distribution_y};

    const auto [global_coloring, coloring_0] = reshuffle::internal::get_global_and_local_coloring(
            current_coloring, data_distributions, 0);
    EXPECT_THAT(coloring_0, Eq(std::vector{0, 1, 2, 3}));
}

TEST(GetGlobalAndLocalColoring, ThrowsIfSizeGlobalColoringDoesNotMatchDimensionsDataDistribution) {
    constexpr int num_values_y = 4;
    constexpr int num_values_x = num_values_y;
    constexpr auto global_coloring = std::vector<reshuffle::rank_id>{};
    const auto data_distributions = std::array{reshuffle::make_block_wise(num_values_x, 1),
                                               reshuffle::make_block_wise(num_values_y, 2)};

    EXPECT_THROW(reshuffle::internal::get_global_and_local_coloring(global_coloring,
                                                                    data_distributions, 0),
                 std::invalid_argument);
}

TEST(GetGlobalAndLocalColoring,
     ThrowsIfNumberOfValuesDataDistributionNotTheSameAsSizeGlobalColoring) {
    constexpr int num_values = 10;
    constexpr auto global_coloring = std::vector<reshuffle::rank_id>{};
    const auto data_distribution = reshuffle::make_block_wise(num_values, 1);

    EXPECT_THROW(reshuffle::internal::get_global_and_local_coloring(global_coloring,
                                                                    data_distribution, 0),
                 std::invalid_argument);
}

TEST(GetGlobalAndLocalColoring,
     ThrowsIn2DIfDimensionsDataDistributionNotEqualToSizeGlobalColoring) {
    constexpr int num_values_y = 4;
    constexpr int num_values_x = num_values_y;
    constexpr auto global_coloring = std::vector<reshuffle::rank_id>{};
    const auto data_distributions = std::array{reshuffle::make_block_wise(num_values_x, 1),
                                               reshuffle::make_block_wise(num_values_y, 2)};

    EXPECT_THROW(reshuffle::internal::get_global_and_local_coloring(global_coloring,
                                                                    data_distributions, 0),
                 std::invalid_argument);
}

TEST(GetGlobalColoring, ReturnsGlobalColoringIn1D) {
    constexpr int num_values = 4;
    const auto data_distribution = reshuffle::make_block_wise(num_values, 2);

    const auto global_coloring = reshuffle::internal::get_global_coloring(data_distribution);
    EXPECT_THAT(global_coloring, Eq(std::vector<reshuffle::rank_id>{0, 0, 1, 1}));
}

TEST(GetGlobalColoring, ReturnsGlobalColoringIn2D) {
    constexpr int num_values_y = 2;
    constexpr int num_values_x = 2;
    const auto data_distributions = std::array{reshuffle::make_block_wise(num_values_x, 1),
                                               reshuffle::make_block_wise(num_values_y, 2)};

    const auto global_coloring = reshuffle::internal::get_global_coloring(data_distributions);
    EXPECT_THAT(global_coloring, Eq(std::vector<reshuffle::rank_id>{0, 0, 1, 1}));
}


TEST(GetBlockDimensions, In1DReturnsTheNumberOfValues) {
    constexpr int num_values = 5;
    const auto data_distribution = reshuffle::make_block_wise(num_values, 2);

    const auto num_values_0 = reshuffle::internal::get_block_dimension(data_distribution, 0);
    const auto num_values_1 = reshuffle::internal::get_block_dimension(data_distribution, 1);
    const auto num_values_2 = reshuffle::internal::get_block_dimension(data_distribution, 2);

    EXPECT_THAT(num_values_0, Eq(3));
    EXPECT_THAT(num_values_1, Eq(2));
    EXPECT_THAT(num_values_2, Eq(0));
}

TEST(GetBlockDimensions, In2DReturnsTheBlockDimension) {
    constexpr int num_values_y = 20;
    constexpr int num_values_x = 20;

    const auto data_distribution_x = reshuffle::make_block_wise(num_values_x, 2);
    const auto data_distribution_y = reshuffle::make_block_wise(num_values_y, 1);

    const auto data_distributions = std::array{data_distribution_x, data_distribution_y};


    const auto dimensions_0 = reshuffle::internal::get_block_dimension(data_distributions, 0);

    EXPECT_THAT(dimensions_0[0], Eq(10));
    EXPECT_THAT(dimensions_0[1], Eq(20));
}
