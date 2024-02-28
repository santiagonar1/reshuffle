#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mpi.h>
#include <vector>
#include <reshuffle.hpp>

using ::testing::Eq;

TEST(Shuffle, SplitsDataEquallyAmongRanks) {
    int rank{};
    int num_ranks{};
    constexpr int expected_elements_per_rank = 10;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    std::vector<int> global_values{};
    if (rank == 0) {
        global_values.resize(num_ranks * expected_elements_per_rank);
    }

    auto my_values = reshuffle::shuffle(global_values, MPI_COMM_WORLD);
    EXPECT_THAT(my_values.size(), Eq(expected_elements_per_rank));
}

TEST(Shuffle, WorksForDifferentDatatypes) {
    int rank{};
    int num_ranks{};
    constexpr int expected_elements_per_rank = 10;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    std::vector<double> global_values_d{};
    if (rank == 0) {
        global_values_d.resize(num_ranks * expected_elements_per_rank);
    }

    auto my_values_d = reshuffle::shuffle(global_values_d, MPI_COMM_WORLD);
    EXPECT_THAT(expected_elements_per_rank, Eq(my_values_d.size()));
}