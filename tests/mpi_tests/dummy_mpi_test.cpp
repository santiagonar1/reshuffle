#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mpi.h>

using ::testing::Ge;

TEST(DummyMPITest, TestingMPIEnvironmentWorks) {
    int rank{};
    int size{};

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    EXPECT_THAT(size, Ge(2));

    if (rank == 0) {
        MPI_Send(&rank, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        MPI_Recv(&rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
                 MPI_STATUSES_IGNORE);
    }

    EXPECT_THAT(rank, Ge(0));
}