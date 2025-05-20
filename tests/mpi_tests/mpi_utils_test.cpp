#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mpi.h>
#include <mpi_comm_utils.hpp>
#include <mpi_utils.hpp>

using namespace reshuffle::internal;
using namespace reshuffle::mpi;

using testing::Eq;

TEST(ToMPIDatatype, ConvertsDatatypeToMPIDatatype) {
    EXPECT_THAT(to_mpi_datatype<int>(), Eq(MPI_INT));
    EXPECT_THAT(to_mpi_datatype<float>(), Eq(MPI_FLOAT));
    EXPECT_THAT(to_mpi_datatype<double>(), Eq(MPI_DOUBLE));
}

TEST(ToMPIDatatype, ThrowsIfDatatypeCannotBeConverted) {
    EXPECT_THROW(auto _ = to_mpi_datatype<char>(), std::invalid_argument);
}

TEST(GetRankId, ReturnsRankId) {
    auto rank{MPI_ERR_RANK};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    EXPECT_THAT(get_rank_id(MPI_COMM_WORLD), Eq(rank));
}

TEST(GetRankId, ThrowsIfCommIsNull) {
    EXPECT_THROW(auto _ = get_rank_id(MPI_COMM_NULL), std::invalid_argument);
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

TEST(GetSubComm, CreatesAnMPISubcommunicator) {
    auto comm_rank_0 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(get_num_ranks(comm_rank_0), Eq(1));
        MPI_Comm_free(&comm_rank_0);
    }
}

TEST(BelongsToComm, ReturnsTrueIfRankInCommunicator) {
    const auto comm_rank_0 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));

    if (is_root(MPI_COMM_WORLD)) { EXPECT_TRUE(reshuffle::mpi::belongs_to_comm(comm_rank_0)); }
}

TEST(BelongsToComm, ReturnsFalseIfRankNotInCommunicator) {
    const auto comm_rank_1 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));

    if (is_root(MPI_COMM_WORLD)) { EXPECT_FALSE(reshuffle::mpi::belongs_to_comm(comm_rank_1)); }
}

TEST(BelongsToComm, ReturnsFalseIfMPICommNullPassed) {
    EXPECT_FALSE(reshuffle::mpi::belongs_to_comm(MPI_COMM_NULL));
}

TEST(IsSubComm, ReturnsTrueIfSubComm) {
    const auto comm_rank_0 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    EXPECT_TRUE(reshuffle::mpi::is_sub_comm(MPI_COMM_WORLD, comm_rank_0));
}

TEST(IsSubComm, ReturnsFalseIfNotSubComm) {
    const auto comm_rank_0 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto comm_rank_1 = reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));
    EXPECT_FALSE(reshuffle::mpi::is_sub_comm(comm_rank_0, comm_rank_1));
}

TEST(IsSubComm, CommunicatorIsSubCommOfItself) {
    EXPECT_TRUE(reshuffle::mpi::is_sub_comm(MPI_COMM_WORLD, MPI_COMM_WORLD));
}