#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utils.hpp>

using testing::Eq;
using testing::FieldsAre;
using namespace reshuffle::internal;

TEST(CartesianProduct, CalculatesTheCartesianProductOfTwoVectors) {
    const auto v1 = std::vector{1, 2};
    const auto v2 = std::vector{3, 4};

    EXPECT_THAT(cartesian_product(v1, v2), Eq(std::vector{std::pair{1, 3}, std::pair{2, 3},
                                                          std::pair{1, 4}, std::pair{2, 4}}));
}

TEST(ToMatrix, ConstructsAMatrixFromA1DArray) {
    constexpr auto dimensions = Dimension<2>{2, 2};
    const auto v = std::vector{1, 2, 3, 4};

    EXPECT_THAT(to_matrix(v, dimensions), Eq(std::vector{std::vector{1, 2}, std::vector{3, 4}}));
}

TEST(HaveSameNumValues, ReturnsTrueIfTwoDistributionsHaveSameNumberOfValues) {
    constexpr auto block_size = 2;
    constexpr auto num_values = 6;
    constexpr auto num_ranks = 1;

    const auto d1 = reshuffle::BlockCyclic(block_size, num_values, num_ranks);
    const auto d2 = reshuffle::BlockCyclic(block_size, num_values, num_ranks);

    EXPECT_TRUE(have_same_num_values(d1, d2));
}

TEST(HaveSameNumValues, ReturnsFalseIfTwoDistributionsHaveSameNumberOfValues) {
    constexpr auto block_size = 2;
    constexpr auto num_values = 6;
    constexpr auto num_ranks = 1;

    const auto d1 = reshuffle::BlockCyclic(block_size, num_values, num_ranks);
    const auto d2 = reshuffle::BlockCyclic(block_size, num_values + 1, num_ranks);

    EXPECT_FALSE(have_same_num_values(d1, d2));
}

TEST(HaveSameNumValues, WorksFor2DArrays) {
    constexpr auto block_size = 2;
    constexpr auto num_values = 6;
    constexpr auto num_ranks = 1;

    const auto d1 = std::array{reshuffle::BlockCyclic(block_size, num_values, num_ranks),
                               reshuffle::BlockCyclic(block_size, num_values, num_ranks)};
    const auto d2 = std::array{reshuffle::BlockCyclic(block_size, num_values, num_ranks),
                               reshuffle::BlockCyclic(block_size, num_values, num_ranks)};
    const auto d3 = std::array{reshuffle::BlockCyclic(block_size, num_values + 1, num_ranks),
                               reshuffle::BlockCyclic(block_size, num_values, num_ranks)};

    EXPECT_TRUE(have_same_num_values(d1, d2));
    EXPECT_FALSE(have_same_num_values(d1, d3));
}

TEST(NumValuesInRank, ReturnsTheNumberOfValuesInARankFor2DDistribution) {
    constexpr auto num_columns = 2;
    constexpr auto num_rows = 2;

    const auto distribution = std::array{reshuffle::make_block_wise(num_columns, 2),
                                         reshuffle::make_block_wise(num_rows, 1)};

    EXPECT_THAT(num_values_in_rank(distribution, 0), Eq(2));
    EXPECT_THAT(num_values_in_rank(distribution, 1), Eq(2));
}

TEST(NumberOfValuesInRank, Returns0ForOutOfScopeRank) {
    constexpr auto num_columns = 2;
    constexpr auto num_rows = 2;

    const auto distribution = std::array{reshuffle::make_block_wise(num_columns, 2),
                                         reshuffle::make_block_wise(num_rows, 1)};

    EXPECT_THAT(num_values_in_rank(distribution, 100), Eq(0));
}

TEST(NumValuesInRank, ThrowsIfNegativeRankIsPassed) {
    constexpr auto num_columns = 2;
    constexpr auto num_rows = 2;

    const auto distribution = std::array{reshuffle::make_block_wise(num_columns, 2),
                                         reshuffle::make_block_wise(num_rows, 1)};

    EXPECT_THROW(auto _ = num_values_in_rank(distribution, -1), std::invalid_argument);
}

TEST(NumRanks, ReturnsTheTotalNumberOfRanksIn2DDistribution) {
    constexpr auto num_ranks_x = 2;
    constexpr auto num_ranks_y = 3;
    constexpr auto num_columns = 6;
    constexpr auto num_rows = 6;

    const auto distribution = std::array{reshuffle::make_block_wise(num_columns, num_ranks_x),
                                         reshuffle::make_block_wise(num_rows, num_ranks_y)};

    EXPECT_THAT(num_ranks(distribution), Eq(num_ranks_x * num_ranks_y));
}

TEST(NumElements, ReturnsTheNumberOfElementsOfA2DMatrix) {
    const auto matrix = std::vector{std::vector{1, 2}, std::vector{3, 4}};

    EXPECT_THAT(num_elements(matrix), Eq(4));
}

TEST(NumElements, Returns0ForEmptyMatrix) {
    constexpr auto matrix = std::vector<std::vector<int>>{};

    EXPECT_THAT(num_elements(matrix), Eq(0));
}

TEST(Get2DCoordiantes, Transforms1DIndexInto2DCoordiantes) {
    constexpr auto num_columns = 3;
    constexpr auto origin = 0;

    EXPECT_THAT(get_2d_coordinates(num_columns, origin), FieldsAre(0, 0));
}

TEST(Get2DCoordiantes, UsesRowMajorCounting) {
    constexpr auto num_columns = 3;
    constexpr auto first_element_second_row = num_columns;

    for (int i = 0; i < num_columns; i++) { EXPECT_THAT(get_2d_coordinates(num_columns, i), FieldsAre(i, 0)); }

    EXPECT_THAT(get_2d_coordinates(num_columns, first_element_second_row), FieldsAre(0, 1));
}