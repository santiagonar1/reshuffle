#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <reshuffle.hpp>

using ::testing::Eq;

TEST(CreateColoring, ReturnsColoringPerRankAndGlobalColoring) {
    // i.e., rank 0 has elements 1, 3, 4, and rank 1 elements 0, 2.
    const auto current_coloring = std::vector{1, 0, 1, 0, 0};
    const auto num_values = static_cast<int>(current_coloring.size());
    const auto data_distribution = reshuffle::make_block_wise(num_values, 2);

    const auto [global_coloring, coloring_0] =
            reshuffle::create_coloring(current_coloring, data_distribution, 0);
    const auto [_, coloring_1] = reshuffle::create_coloring(current_coloring, data_distribution, 1);

    EXPECT_THAT(coloring_0, Eq(std::vector{0, 1, 1}));
    EXPECT_THAT(coloring_1, Eq(std::vector{0, 0}));
    EXPECT_THAT(global_coloring, Eq(std::vector{0, 0, 0, 1, 1}));
}

TEST(CreateColoring, CanUseBlockWiseDataDistributionInTwoDimensions) {
    // i.e., 4 values in a 2x2 matrix, previously all in rank 0.
    const auto current_coloring = std::vector<int>(4);
    const auto data_distribution_x = reshuffle::make_block_wise(2, 2);
    const auto data_distribution_y = reshuffle::make_block_wise(2, 2);

    const auto data_distributions = std::array{data_distribution_x, data_distribution_y};
    const auto global_dimensions = reshuffle::Dimensions2D{2, 2};

    const auto [global_coloring, coloring_0] =
            reshuffle::create_coloring(current_coloring, global_dimensions, data_distributions, 0);
    EXPECT_THAT(coloring_0, Eq(std::vector{0, 1, 2, 3}));
}

TEST(CreateColoring, ABlockWiseWithOneBlockIndicatesNoDivision) {
    // i.e., 4 values in a 2x2 matrix, previously all in rank 0.
    const auto global_dimensions = reshuffle::Dimensions2D{2, 2};
    const auto num_values = global_dimensions.num_rows * global_dimensions.num_columns;

    const auto current_coloring = std::vector<int>(num_values);
    const auto data_distribution_x = reshuffle::make_block_wise(global_dimensions.num_columns, 1);
    const auto data_distribution_y = reshuffle::make_block_wise(global_dimensions.num_rows, 2);

    const auto data_distributions = std::array{data_distribution_x, data_distribution_y};

    const auto [global_coloring, coloring_0] =
            reshuffle::create_coloring(current_coloring, global_dimensions, data_distributions, 0);
    EXPECT_THAT(coloring_0, Eq(std::vector{0, 0, 1, 1}));
}

TEST(CreateColoring, ThrowsIfGlobalColoringSizeDoesNotMatchGlobalDimension) {
    constexpr int num_rows = 4;
    constexpr int num_columns = num_rows;
    const auto global_dimensions = reshuffle::Dimensions2D{num_rows, num_columns};
    const auto global_coloring = std::vector<reshuffle::rank_id>{};
    const auto data_distributions = std::array{reshuffle::make_block_wise(num_columns, 1),
                                               reshuffle::make_block_wise(num_rows, 2)};

    EXPECT_THROW(
            reshuffle::create_coloring(global_coloring, global_dimensions, data_distributions, 0),
            std::invalid_argument);
}

TEST(CreateColoring, In1DNumberOfValuesDistributionMustBeTheSameAsSizeGlobalColoring) {
    constexpr int num_values = 10;
    const auto global_coloring = std::vector<reshuffle::rank_id>{};
    const auto data_distribution = reshuffle::make_block_wise(num_values, 1);

    EXPECT_THROW(reshuffle::create_coloring(global_coloring, data_distribution, 0),
                 std::invalid_argument);
}

TEST(CreateColoring, NumberOfValuesDistributionMustBeEqualToSizeGlobalColoring) {
    constexpr int num_rows = 4;
    constexpr int num_columns = num_rows;
    const auto global_dimensions = reshuffle::Dimensions2D{num_rows, num_columns};
    const auto global_coloring = std::vector<reshuffle::rank_id>{};
    const auto data_distributions = std::array{reshuffle::make_block_wise(num_columns, 1),
                                               reshuffle::make_block_wise(num_rows, 2)};

    EXPECT_THROW(
            reshuffle::create_coloring(global_coloring, global_dimensions, data_distributions, 0),
            std::invalid_argument);
}


TEST(CreateColoring, NumberOfValuesFirstElementOfDistributionMustBeEqualToNumberOfColumns) {
    constexpr int num_rows = 4;
    constexpr int num_columns = num_rows * 2;
    const auto global_dimensions = reshuffle::Dimensions2D{num_rows, num_columns};
    const auto global_coloring = std::vector<reshuffle::rank_id>(num_rows * num_columns);
    const auto data_distributions = std::array{reshuffle::make_block_wise(num_rows, 1),
                                               reshuffle::make_block_wise(num_rows, 2)};

    EXPECT_THROW(
            reshuffle::create_coloring(global_coloring, global_dimensions, data_distributions, 0),
            std::invalid_argument);
}

TEST(CreateColoring, NumberOfValuesSecondElementOfDistributionMustBeEqualToNumberOfRows) {
    constexpr int num_rows = 4;
    constexpr int num_columns = num_rows * 2;
    const auto global_dimensions = reshuffle::Dimensions2D{num_rows, num_columns};
    const auto global_coloring = std::vector<reshuffle::rank_id>(num_rows * num_columns);
    const auto data_distributions = std::array{reshuffle::make_block_wise(num_columns, 1),
                                               reshuffle::make_block_wise(num_columns, 2)};

    EXPECT_THROW(
            reshuffle::create_coloring(global_coloring, global_dimensions, data_distributions, 0),
            std::invalid_argument);
}


TEST(GetBlockDimensions, In1DReturnsTheNumberOfValues) {
    constexpr int num_values = 5;
    const auto data_distribution = reshuffle::make_block_wise(num_values, 2);

    const auto num_values_0 = reshuffle::get_block_dimension(data_distribution, 0);
    const auto num_values_1 = reshuffle::get_block_dimension(data_distribution, 1);
    const auto num_values_2 = reshuffle::get_block_dimension(data_distribution, 2);

    EXPECT_THAT(num_values_0, Eq(3));
    EXPECT_THAT(num_values_1, Eq(2));
    EXPECT_THAT(num_values_2, Eq(0));
}

TEST(GetBlockDimensions, In2DReturnsTheBlockDimension) {
    const auto global_dimensions = reshuffle::Dimensions2D{20, 20};

    const auto data_distribution_x = reshuffle::make_block_wise(global_dimensions.num_columns, 2);
    const auto data_distribution_y = reshuffle::make_block_wise(global_dimensions.num_rows, 1);

    const auto data_distributions = std::array{data_distribution_x, data_distribution_y};


    const auto dimensions_0 = reshuffle::get_block_dimension(data_distributions, 0);

    EXPECT_THAT(dimensions_0.num_columns, Eq(10));
    EXPECT_THAT(dimensions_0.num_rows, Eq(20));
}
