#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <greedy_rank_order_strategy.hpp>
#include <hungarian_rank_order_strategy.hpp>
#include <rank_order.hpp>


using testing::Eq;

TEST(CommunicationWeight, CommunicationWeightCanHandleOneToTwoProcessors) {

    constexpr auto num_global_values = 6;

    const auto initial_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{1}});

    const auto final_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{2}});

    const auto rank_order =
            reshuffle::internal::RankOrder{initial_distribution, final_distribution,
                                           reshuffle::internal::GreedyRankOrderStrategy{}};
    const auto &matrix = rank_order.get_matrix();

    //1X2 Matrix
    EXPECT_EQ(matrix.size(), 1);
    EXPECT_EQ(matrix[0].size(), 2);

    //We are redistributing from 1 rank to 2 ranks in one dimension.
    //rank 0 should send 3 values to rank 0 and 3 values to rank 1
    EXPECT_THAT(matrix[0], Eq(std::vector{3, 3}));
}


TEST(CommunicationWeight, CommunicationWeightCanHandleTwoToOneProcessors) {

    constexpr auto num_global_values = 6;

    const auto initial_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{2}});

    const auto final_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{1}});

    const auto rank_order =
            reshuffle::internal::RankOrder{initial_distribution, final_distribution,
                                           reshuffle::internal::GreedyRankOrderStrategy{}};
    const auto &matrix = rank_order.get_matrix();

    //2X1 Matrix
    EXPECT_EQ(matrix.size(), 2);
    EXPECT_EQ(matrix[0].size(), 1);
    EXPECT_EQ(matrix[1].size(), 1);

    //We are redistributing from 2 ranks to 1 rank in one dimension.
    //rank 0 should send 3 values to rank 0, and rank 1 should send 3 values to rank 0
    EXPECT_EQ(matrix[0][0], 3);
    EXPECT_EQ(matrix[1][0], 3);
}


TEST(CommunicationWeight, CommunicationWeightCanHandleUnevenDistribution) {
    constexpr auto num_global_values = 11;

    const auto initial_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{3}});

    const auto final_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{2}});

    const auto rank_order =
            reshuffle::internal::RankOrder{initial_distribution, final_distribution,
                                           reshuffle::internal::GreedyRankOrderStrategy{}};
    const auto &matrix = rank_order.get_matrix();

    //3X2 Matrix
    EXPECT_EQ(matrix.size(), 3);
    EXPECT_EQ(matrix[0].size(), 2);

    EXPECT_THAT(matrix[0], Eq(std::vector{4, 0}));
    EXPECT_THAT(matrix[1], Eq(std::vector{2, 2}));
    EXPECT_THAT(matrix[2], Eq(std::vector{0, 3}));
}


TEST(CommunicationWeight, CommunicationWeightCanHandle2DDistribution) {
    constexpr auto num_global_values_x = 4;
    constexpr auto num_global_values_y = 4;

    const auto initial_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values_x, num_global_values_y}, reshuffle::ProcessorGrid<2>{{2, 2}});

    const auto final_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values_x, num_global_values_y}, reshuffle::ProcessorGrid<2>{{2, 2}});

    const auto rank_order =
            reshuffle::internal::RankOrder<2>{initial_distribution, final_distribution,
                                              reshuffle::internal::GreedyRankOrderStrategy{}};
    const auto &matrix = rank_order.get_matrix();

    //4X4 Matirx
    EXPECT_EQ(matrix.size(), 4);
    EXPECT_EQ(matrix[0].size(), 4);


    EXPECT_THAT(matrix[0], Eq(std::vector{4, 0, 0, 0}));
    EXPECT_THAT(matrix[1], Eq(std::vector{0, 4, 0, 0}));
    EXPECT_THAT(matrix[2], Eq(std::vector{0, 0, 4, 0}));
    EXPECT_THAT(matrix[3], Eq(std::vector{0, 0, 0, 4}));
}


TEST(CommunicationWeight, CommunicationWeightCanHandleUneven2DDistribution) {
    constexpr auto num_global_values_x = 5;
    constexpr auto num_global_values_y = 3;

    const auto initial_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values_x, num_global_values_y}, reshuffle::ProcessorGrid<2>{{2, 2}});

    const auto final_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values_x, num_global_values_y}, reshuffle::ProcessorGrid<2>{{1, 2}});

    const auto rank_order =
            reshuffle::internal::RankOrder<2>{initial_distribution, final_distribution,
                                              reshuffle::internal::GreedyRankOrderStrategy{}};
    const auto &matrix = rank_order.get_matrix();

    //4X2 Matrix
    EXPECT_EQ(matrix.size(), 4);
    EXPECT_EQ(matrix[0].size(), 2);

    EXPECT_THAT(matrix[0], Eq(std::vector{6, 0}));
    EXPECT_THAT(matrix[1], Eq(std::vector{0, 3}));
    EXPECT_THAT(matrix[2], Eq(std::vector{4, 0}));
    EXPECT_THAT(matrix[3], Eq(std::vector{0, 2}));
}


TEST(compute_optimal_rank_order, CanHandleMoreColsThanRows) {
    constexpr auto num_global_values = 8;

    const auto initial_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{4}});

    const auto final_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{4}});

    const auto hungarian_strategy = reshuffle::internal::HungarianRankOrderStrategy{};
    const auto greedy_strategy = reshuffle::internal::GreedyRankOrderStrategy{};

    auto rank_order_greedy = reshuffle::internal::RankOrder{initial_distribution,
                                                            final_distribution, greedy_strategy};

    auto rank_order_hungarian = reshuffle::internal::RankOrder{
            initial_distribution, final_distribution, hungarian_strategy};

    rank_order_greedy._test_set_matrix(
            {{2, 1, 8, 1, 9, 2}, {1, 2, 1, 8, 3, 10}, {8, 1, 2, 1, 2, 3}, {1, 8, 1, 2, 1, 3}});

    rank_order_hungarian._test_set_matrix(
            {{2, 1, 8, 1, 9, 2}, {1, 2, 1, 8, 3, 10}, {8, 1, 2, 1, 2, 3}, {1, 8, 1, 2, 1, 3}});

    const auto greedy_permutation = rank_order_greedy.get_optimal_rank_order();
    const auto hung_permutation = rank_order_hungarian.get_optimal_rank_order();
    const auto expected_permutation = std::vector{4, 5, 0, 1, 2, 3};

    EXPECT_EQ(expected_permutation, greedy_permutation);
    EXPECT_EQ(expected_permutation, hung_permutation);
}

TEST(compute_optimal_rank_order, CanHandleMoreRowsThanCols) {
    constexpr auto num_global_values = 8;

    const auto initial_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{4}});

    const auto final_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{4}});

    const auto hungarian_strategy = reshuffle::internal::HungarianRankOrderStrategy{};
    const auto greedy_strategy = reshuffle::internal::GreedyRankOrderStrategy{};

    auto rank_order_greedy = reshuffle::internal::RankOrder{initial_distribution,
                                                            final_distribution, greedy_strategy};

    auto rank_order_hungarian = reshuffle::internal::RankOrder{
            initial_distribution, final_distribution, hungarian_strategy};

    rank_order_greedy._test_set_matrix(
            {{2, 1, 8, 1}, {1, 2, 1, 8}, {8, 1, 2, 1}, {1, 8, 1, 2}, {2, 1, 10, 1}});

    rank_order_hungarian._test_set_matrix(
            {{2, 1, 8, 1}, {1, 2, 1, 8}, {8, 1, 2, 1}, {1, 8, 1, 2}, {2, 1, 10, 1}});

    const auto greedy_permutation = rank_order_greedy.get_optimal_rank_order();
    const auto hung_permutation = rank_order_hungarian.get_optimal_rank_order();
    const auto expected_permutation = std::vector{2, 3, 0, 1, 4};

    EXPECT_EQ(expected_permutation, greedy_permutation);
    EXPECT_EQ(expected_permutation, hung_permutation);
}

TEST(compute_optimal_rank_order, GreedyGivesFirstAndHungarianGivesBestSolution) {

    constexpr auto num_global_values = 8;

    const auto initial_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{4}});

    const auto final_distribution = reshuffle::make_block_wise_distribution(
            {num_global_values}, reshuffle::ProcessorGrid<1>{{4}});

    const auto hungarian_strategy = reshuffle::internal::HungarianRankOrderStrategy{};
    const auto greedy_strategy = reshuffle::internal::GreedyRankOrderStrategy{};

    auto rank_order_greedy = reshuffle::internal::RankOrder{initial_distribution,
                                                            final_distribution, greedy_strategy};

    auto rank_order_hungarian = reshuffle::internal::RankOrder{
            initial_distribution, final_distribution, hungarian_strategy};

    rank_order_greedy._test_set_matrix({{9, 8, 0}, {9, 1, 0}, {0, 0, 1}});

    rank_order_hungarian._test_set_matrix({{9, 8, 0}, {9, 1, 0}, {0, 0, 1}});

    const auto greedy_permutation = rank_order_greedy.get_optimal_rank_order();
    const auto hung_permutation = rank_order_hungarian.get_optimal_rank_order();

    const auto expected_greedy_permutation = std::vector{0, 1, 2};
    const auto expected_hung_permutation = std::vector{1, 0, 2};

    EXPECT_EQ(expected_greedy_permutation, greedy_permutation);
    EXPECT_EQ(expected_hung_permutation, hung_permutation);
}
