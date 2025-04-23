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
    constexpr auto dimensions = Dimensions<2>{2, 2};
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
    constexpr auto num_values_x = 2;
    constexpr auto num_values_y = 2;

    const auto distribution = std::array{reshuffle::make_block_wise(num_values_x, 2),
                                         reshuffle::make_block_wise(num_values_y, 1)};

    EXPECT_THAT(num_values_in_rank(distribution, 0), Eq(2));
    EXPECT_THAT(num_values_in_rank(distribution, 1), Eq(2));
}

TEST(NumValuesInRank, WorksForSplitInMultipleDimensions) {
    constexpr int num_values_x = 2;
    constexpr int num_values_y = 2;

    const auto distribution = std::array{reshuffle::make_block_wise(num_values_x, 2),
                                         reshuffle::make_block_wise(num_values_y, 2)};

    EXPECT_THAT(num_values_in_rank(distribution, 0), Eq(1));
    EXPECT_THAT(num_values_in_rank(distribution, 1), Eq(1));
    EXPECT_THAT(num_values_in_rank(distribution, 2), Eq(1));
    EXPECT_THAT(num_values_in_rank(distribution, 3), Eq(1));
}

TEST(NumberOfValuesInRank, Returns0ForOutOfScopeRank) {
    constexpr auto num_values_x = 2;
    constexpr auto num_valuex_y = 2;

    const auto distribution = std::array{reshuffle::make_block_wise(num_values_x, 2),
                                         reshuffle::make_block_wise(num_valuex_y, 1)};

    EXPECT_THAT(num_values_in_rank(distribution, 100), Eq(0));
}

TEST(NumValuesInRank, ThrowsIfNegativeRankIsPassed) {
    constexpr auto num_values_x = 2;
    constexpr auto num_values_y = 2;

    const auto distribution = std::array{reshuffle::make_block_wise(num_values_x, 2),
                                         reshuffle::make_block_wise(num_values_y, 1)};

    EXPECT_THROW(auto _ = num_values_in_rank(distribution, -1), std::invalid_argument);
}

TEST(NumRanks, ReturnsTheTotalNumberOfRanksIn2DDistribution) {
    constexpr auto num_ranks_x = 2;
    constexpr auto num_ranks_y = 3;
    constexpr auto num_values_x = 6;
    constexpr auto num_values_y = 6;

    const auto distribution = std::array{reshuffle::make_block_wise(num_values_x, num_ranks_x),
                                         reshuffle::make_block_wise(num_values_y, num_ranks_y)};

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
    constexpr auto num_values_x = 3;
    constexpr auto origin = 0;

    EXPECT_THAT(get_2d_coordinates(num_values_x, origin), FieldsAre(0, 0));
}

TEST(Get2DCoordiantes, UsesRowMajorCounting) {
    constexpr auto num_values_x = 3;
    constexpr auto first_element_second_row = num_values_x;

    for (int i = 0; i < num_values_x; i++) {
        EXPECT_THAT(get_2d_coordinates(num_values_x, i), FieldsAre(i, 0));
    }

    EXPECT_THAT(get_2d_coordinates(num_values_x, first_element_second_row), FieldsAre(0, 1));
}

TEST(GetValues, ReturnsTheListOfValuesListedInTheIndicesVector) {
    const auto values = std::vector{0, 1, 2, 3, 4};
    const auto indices = std::vector{0, 4};

    EXPECT_THAT(get_values(values, indices), Eq(std::vector{0, 4}));
}

TEST(GetValues, ThrowsIfIndexOutOfBounds) {
    const auto values = std::vector{0, 1, 2, 3, 4};
    const auto indices = std::vector{5};

    EXPECT_THROW(auto _ = get_values(values, indices), std::out_of_range);
}

TEST(ReorderValues, ReturnsVectorOfValuesOrderdedBasedOnNewIndices) {
    const auto values = std::vector{0, 1, 2, 3, 4};
    const auto new_indices = std::vector{3, 2, 0, 4, 1};

    EXPECT_THAT(reorder_values(values, new_indices), Eq(std::vector{2, 4, 1, 0, 3}));
}

TEST(ReorderValues, ThrowsIfSizeOfNewIndicesDifferentFromSizeOfValues) {
    const auto values = std::vector{0, 1, 2, 3, 4};
    const auto less_indices = std::vector{0};
    const auto more_indices = std::vector{0, 1, 2, 3, 4, 5};

    EXPECT_THROW(auto _ = reorder_values(values, less_indices), std::invalid_argument);
    EXPECT_THROW(auto _ = reorder_values(values, more_indices), std::invalid_argument);
}

TEST(GroupValuesByRankId, GroupsAllValuesThatHaveSameAssociatedRankIdTogether) {
    const auto values = std::vector{0, 1, 2, 3, 4};
    const auto rank_ids = std::vector{0, 1, 0, 0, 2};
    constexpr auto num_ranks = 3;

    // 0, 2, 3 -> rank 0, 1 -> rank 1, 4 -> rank 2
    EXPECT_THAT(group_values_by_rank_id(values, rank_ids, num_ranks),
                Eq(std::vector{0, 2, 3, 1, 4}));
}

TEST(GroupValuesByRankId, ThrowsIfSizeOfAssociatedRankIdsDifferentFromSizeOfValues) {
    const auto values = std::vector{0, 1, 2, 3, 4};
    const auto less_indices = std::vector{0};
    const auto more_indices = std::vector{0, 1, 2, 3, 4, 5};

    EXPECT_THROW(auto _ = group_values_by_rank_id(values, less_indices, 5), std::invalid_argument);
    EXPECT_THROW(auto _ = group_values_by_rank_id(values, more_indices, 5), std::invalid_argument);
}

TEST(GetNumRepetitions, CalculatesHowOftenANumberRepeatsAndStoreInIndex) {
    const auto values = std::vector{0, 0, 2, 3, 2, 2};
    constexpr auto max_value = 3;

    // 0: 2 times, 1: 0 times, 2: 3 times, 3: 1 time
    EXPECT_THAT(get_num_repetitions(values, max_value), Eq(std::vector{2, 0, 3, 1}));
}