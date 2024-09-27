#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mpi.h>
#include <mpi_utils.hpp>

using namespace reshuffle::internal;
using testing::Eq;

TEST(ToMPIDatatype, ConvertsDatatypeToMPIDatatype) {
    EXPECT_THAT(to_mpi_datatype<int>(), Eq(MPI_INT));
    EXPECT_THAT(to_mpi_datatype<float>(), Eq(MPI_FLOAT));
    EXPECT_THAT(to_mpi_datatype<double>(), Eq(MPI_DOUBLE));
}

TEST(ToMPIDatatype, ThrowsIfDatatypeCannotBeConverted) {
    EXPECT_THROW(to_mpi_datatype<char>(), std::invalid_argument);
}

TEST(GetRankId, ReturnsRankId) {
    auto rank{MPI_ERR_RANK};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    EXPECT_THAT(get_rank_id(MPI_COMM_WORLD), Eq(rank));
}

TEST(GetRankId, ThrowsIfCommIsNull) {
    EXPECT_THROW(auto _ = get_rank_id(MPI_COMM_NULL), std::invalid_argument);
}

TEST(InMPIComm, ReturnsTrueIfCommIsNotNull) {
    const auto comm{MPI_COMM_WORLD};
    EXPECT_TRUE(in_mpi_comm(comm));
}

TEST(InMPIComm, ReturnsFalseIfCommIsNull) {
    const auto comm{MPI_COMM_NULL};
    EXPECT_FALSE(in_mpi_comm(comm));
}

TEST(IsRoot, ReturnsTrueForRankZero) {
    auto rank{MPI_ERR_RANK};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) { EXPECT_TRUE(is_root(MPI_COMM_WORLD)); }
}

TEST(IsRoot, ReturnsFalseIfRankNotZero) {
    auto rank{MPI_ERR_RANK};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank != 0) { EXPECT_FALSE(is_root(MPI_COMM_WORLD)); }
}

TEST(IsRoot, ReturnsFalseIfRankNotInComm) { EXPECT_FALSE(is_root(MPI_COMM_NULL)); }

TEST(GetNumRanks, ReturnsTheNumberOfRanks) {
    auto num_ranks{0};
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    EXPECT_THAT(get_num_ranks(MPI_COMM_WORLD), Eq(num_ranks));
}

TEST(GetNumRanks, ThrowsIfCommIsNull) {
    EXPECT_THROW(auto _ = get_num_ranks(MPI_COMM_NULL), std::invalid_argument);
}

TEST(IsCommNull, ReturnsTrueIfCommIsNull) { EXPECT_TRUE(is_comm_null(MPI_COMM_NULL)); }

TEST(IsCommNull, ReturnsFalseIfCommNotNull) { EXPECT_FALSE(is_comm_null(MPI_COMM_WORLD)); }