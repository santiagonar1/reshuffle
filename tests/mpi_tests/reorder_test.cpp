#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <numeric>

#include <block_wise.hpp>
#include <shuffle.hpp>

using namespace reshuffle;
using namespace reshuffle::mpi;
using namespace reshuffle::distribution;

using testing::Eq;


auto calcValues(auto rank, auto global_values, auto num_ranks) -> std::vector<int> {
    if (rank >= num_ranks) return {};
    auto num_values_per_rank = (global_values + num_ranks - 1) / num_ranks;
    std::vector<int> values;
    if (rank == num_ranks - 1) {
        values = std::vector<int>(global_values - num_values_per_rank * rank);
    } else {
        values = std::vector<int>(num_values_per_rank);
    }
    std::iota(values.begin(), values.end(), rank * num_values_per_rank);
    return values;
}

TEST(get_optimal_communicator, CanRelabelFromLessToMoreProcessors) {
    const auto rank = get_rank_id(MPI_COMM_WORLD).value();
    constexpr auto num_global_values = 18;

    const auto initial_distribution = BlockWise{{num_global_values}, ProcessorGrid{4}};

    const auto final_distribution = BlockWise{{num_global_values}, ProcessorGrid{6}};

    const auto values = calcValues(rank, num_global_values,
                                   initial_distribution.get_processor_grid().get_num_processors());

    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto optimalCommunicator = get_optimal_communicator(initial_context, final_context);

    const auto reorder_final_context =
            Context{final_distribution, optimalCommunicator.value().first};

    const auto new_values = shuffle(std::mdspan{values.data(), values.size()}, initial_context,
                                    reorder_final_context)
                                    .first;

    const std::vector<int> expected_new_values_0 = {0, 1, 2};
    const std::vector<int> expected_new_values_1 = {12, 13, 14};
    const std::vector<int> expected_new_values_2 = {3, 4, 5};
    const std::vector<int> expected_new_values_3 = {15, 16, 17};
    const std::vector<int> expected_new_values_4 = {6, 7, 8};
    const std::vector<int> expected_new_values_5 = {9, 10, 11};

    if (rank == 0) {
        EXPECT_EQ(new_values, expected_new_values_0);
    } else if (rank == 1) {
        EXPECT_EQ(new_values, expected_new_values_1);
    } else if (rank == 2) {
        EXPECT_EQ(new_values, expected_new_values_2);
    } else if (rank == 3) {
        EXPECT_EQ(new_values, expected_new_values_3);
    } else if (rank == 4) {
        EXPECT_EQ(new_values, expected_new_values_4);
    } else if (rank == 5) {
        EXPECT_EQ(new_values, expected_new_values_5);
    }
}

TEST(get_optimal_communicator, CanRelabelFromMoreToLessProcessors) {
    const auto rank = get_rank_id(MPI_COMM_WORLD).value();
    constexpr auto num_global_values = 18;

    const auto initial_distribution = BlockWise{{num_global_values}, ProcessorGrid{6}};

    const auto final_distribution = BlockWise{{num_global_values}, ProcessorGrid{4}};

    const auto values = calcValues(rank, num_global_values,
                                   initial_distribution.get_processor_grid().get_num_processors());

    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto optimalCommunicator = get_optimal_communicator(initial_context, final_context);

    const auto reorder_final_context =
            Context{final_distribution, optimalCommunicator.value().first};

    const auto new_values = shuffle(std::mdspan{values.data(), values.size()}, initial_context,
                                    reorder_final_context)
                                    .first;

    const std::vector<int> expected_new_values_0 = {0, 1, 2, 3, 4};
    const std::vector<int> expected_new_values_1 = {10, 11, 12, 13, 14};
    const std::vector<int> expected_new_values_2 = {15, 16, 17};
    const std::vector<int> expected_new_values_3 = {5, 6, 7, 8, 9};
    constexpr std::vector<int> expected_new_values_4 = {};
    constexpr std::vector<int> expected_new_values_5 = {};

    if (rank == 0) {
        EXPECT_EQ(new_values, expected_new_values_0);
    } else if (rank == 1) {
        EXPECT_EQ(new_values, expected_new_values_1);
    } else if (rank == 2) {
        EXPECT_EQ(new_values, expected_new_values_2);
    } else if (rank == 3) {
        EXPECT_EQ(new_values, expected_new_values_3);
    } else if (rank == 4) {
        EXPECT_EQ(new_values, expected_new_values_4);
    } else if (rank == 5) {
        EXPECT_EQ(new_values, expected_new_values_5);
    }
}